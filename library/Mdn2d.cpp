#include "Mdn2d.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Mdn2d::Mdn2d() :
    Mdn2dRules()
{
    Log_Debug2("Constructing Mdn2d null");
}


mdn::Mdn2d::Mdn2d(Mdn2dConfig config) :
    Mdn2dRules(config)
{
    Log_Debug2("Constructing Mdn2d with config " << config);
}


mdn::Mdn2d::Mdn2d(const Mdn2d& other):
    Mdn2dRules(other)
{
    Log_Debug2("Copy constructing");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    Mdn2dRules::operator=(other);
    Log_Debug2("Copying");
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other) noexcept :
    Mdn2dRules(std::move(other))
{
    Log_Debug2("Move constructor");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    Mdn2dRules::operator=(std::move(other));
    Log_Debug2("Move copying");
    return *this;
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_Debug2("ans = *this + rhs");
    CoordSet changed = locked_plus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_Debug3("ans = *this + rhs");
    CoordSet changed;
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, digit));
    }
    return changed;
}


void mdn::Mdn2d::minus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "minus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_Debug2("ans = *this - rhs");
    CoordSet changed = locked_minus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_Debug3("ans = *this - rhs");
    CoordSet changed;
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, -digit));
    }
    return changed;
}


void mdn::Mdn2d::multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "multiply operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_Debug2("ans = *this * rhs");
    CoordSet changed = locked_multiply(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_Debug3("ans = *this * rhs");
    ans.locked_clear();
    for (const auto& [xy, digit] : rhs.m_raw) {
        int id = static_cast<int>(digit);
        // ans += (this x rhs_id).shift(rhs_xy)
        ans.locked_plusEquals(internal_copyMultiplyAndShift(id, xy));

    }
    return ans.m_index;
}


void mdn::Mdn2d::divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    assertNotSelf(ans, "divide operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    }
    CoordSet changed = locked_divide(rhs, ans, fraxis);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    // TODO
    if (Log_Showing_Debug3) {
        Log_Debug3("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    }
    return CoordSet();
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    internal_checkFraxis(fraxis);
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(realNum), fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_Debug2("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, realNum, fraxis));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, double realNum, Fraxis fraxis) {
    if (Log_Showing_Debug3) {
        Log_Debug3("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    CoordSet changed = locked_add(xy, static_cast<long>(intPart));
    changed.merge(locked_addFraxis(xy, fracPart, fraxis));
    internal_operationComplete();
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_Debug2("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(-realNum), fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_Debug2("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, -realNum, fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2("add " << static_cast<int>(value) << ", no fraxis");
    }
    int ivalue = static_cast<int>(value);
    locked_carryoverCleanup(locked_add(xy, ivalue));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, Digit value) {
    if (Log_Showing_Debug3) {
        Log_Debug3("add " << static_cast<int>(value) << ", no fraxis");
    }
    return locked_add(xy, static_cast<int>(value));
}


void mdn::Mdn2d::add(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, int value) {
    int val = static_cast<int>(locked_getValue(xy));
    int sum = val + value;
    int carry = sum / m_config.base();
    int rem = sum % m_config.base();
    CoordSet changed;
    Log_Debug3(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );

    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.copyTranslateX(1), carry));
        changed.merge(locked_add(xy.copyTranslateY(1), carry));
    }
    return changed;
    internal_operationComplete();
}


void mdn::Mdn2d::add(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long value) {
    long val = static_cast<long>(locked_getValue(xy));
    long sum = val + value;
    long carry = sum / m_config.base();
    long rem = sum % m_config.base();
    Log_Debug3(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.copyTranslateX(1), carry));
        changed.merge(locked_add(xy.copyTranslateY(1), carry));
    }
    return changed;
}


void mdn::Mdn2d::add(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("at " << xy << ", add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long long value) {
    long long val = static_cast<long long>(locked_getValue(xy));
    long long sum = val + value;
    long long carry = sum / m_config.base();
    long long rem = sum % m_config.base();
    Log_Debug3(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.copyTranslateX(1), carry));
        changed.merge(locked_add(xy.copyTranslateY(1), carry));
    }
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2("at " << xy << ", add " << static_cast<int>(value) << ", no fraxis");
    }
    locked_carryoverCleanup(locked_add(xy, -static_cast<int>(value)));
    internal_operationComplete();
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_Debug2("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
}


void mdn::Mdn2d::addFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, static_cast<double>(fraction), fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, fraction, fraxis));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    if (fraction < -1.0 || fraction > 1.0) {
        std::invalid_argument error(
            "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
            "."
        );
        Log_Error(error.what());
        throw error;
    }
    if (Log_Showing_Debug3) {
        Log_Debug3(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }

    CoordSet changed;
    switch(fraxis) {
        case Fraxis::X:
            changed = internal_fraxis(xy.copyTranslateX(-1), fraction, -1, 0, -1);
            break;
        case Fraxis::Y:
            changed = internal_fraxis(xy.copyTranslateY(-1), fraction, 0, -1, 1);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            std::invalid_argument err(oss.str());
            Log_Error(err.what());
            throw err;
            break;
    }
    return changed;
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, -static_cast<double>(fraction), fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, -fraction, fraxis));
    internal_operationComplete();
}


void mdn::Mdn2d::multiply(Digit value) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2("scalar multiply " << static_cast<int>(value));
    }
    locked_carryoverCleanup(locked_multiply(static_cast<int>(value)));
    internal_operationComplete();
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    Log_Debug2("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
}


void mdn::Mdn2d::multiply(long value) {
    auto lock = lockWriteable();
    Log_Debug2("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
}


void mdn::Mdn2d::multiply(long long value) {
    auto lock = lockWriteable();
    Log_Debug2("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(int value) {
    Log_Debug3("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        int vxd = value*static_cast<int>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long value) {
    Log_Debug3("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long vxd = value*static_cast<long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long long value) {
    Log_Debug3("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long long vxd = value*static_cast<long long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    return changed;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    auto lock = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    if (rhs.m_config != m_config) {
        Log_Debug2("equality test, result: config differs");
        return false;
    }
    if (m_raw == rhs.m_raw) {
        Log_Debug2("equality test, result: data are equal");
        return true;
    }
    if (
        m_config.signConvention() == SignConvention::Positive ||
        m_config.signConvention() == SignConvention::Negative
    ) {
        Log_Debug2(
            "equality test, result: data unequal, sign convention in place, therefore \n"
            << "    polymorphism tests unnecessary"
        );
        return false;
    }
    Mdn2d dlhs = Duplicate(*this);
    Mdn2d drhs = Duplicate(rhs);
    auto dlhsLock = dlhs.lockWriteable();
    auto drhsLock = drhs.lockWriteable();
    dlhs.m_config.setSignConvention(SignConvention::Positive);
    drhs.m_config.setSignConvention(SignConvention::Positive);
    dlhs.locked_carryoverCleanupAll();
    drhs.locked_carryoverCleanupAll();
    bool result = dlhs.m_raw == drhs.m_raw;
    Log_Debug2(
        "equality test, result: final polymorphism tests, result = " << result
    );
    return result;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    Log_Debug3("inequality test");
    return !(*this == rhs);
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_Debug2("plus equals");
    CoordSet changed = locked_plusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_plusEquals(const Mdn2d& rhs) {
    Log_Debug3("plus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, digit));
    }
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_Debug2("minus equals");
    CoordSet changed = locked_minusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_minusEquals(const Mdn2d& rhs) {
    Log_Debug3("minus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, -digit));
    }
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_Debug2("times equals (Mdn2d)");
    locked_carryoverCleanup(locked_timesEquals(rhs));
    internal_operationComplete();
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_timesEquals(const Mdn2d& rhs) {
    Log_Debug3("times equals (Mdn2d)");
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_multiply(rhs, temp);
    operator=(temp);
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    Mdn2d temp = Duplicate(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    Log_Debug3("divide equals (Mdn2d)");
    CoordSet changed = locked_divide(rhs, temp);
    operator=(temp);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    auto lock = lockWriteable();
    Log_Debug3("times equals (int scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long scalar) {
    auto lock = lockWriteable();
    Log_Debug3("times equals (long scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long long scalar) {
    auto lock = lockWriteable();
    Log_Debug3("times equals (long long scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    return *this;
}


void mdn::Mdn2d::internal_checkFraxis(Fraxis& fraxis) const {
    if (fraxis == Fraxis::Default) {
        if (Log_Showing_Debug4) {
            Log_Debug4("applying default fraxis: " << FraxisToName(fraxis));
        }
        fraxis = m_config.defaultFraxis();
    }
}


mdn::CoordSet mdn::Mdn2d::internal_fraxis(const Coord& xy, double f, int dX, int dY, int c) {
    #ifdef MDN_DEBUG
        if (f < -1.0 || f > 1.0)
        {
            throw std::invalid_argument(
                "Fractional part must be -1.0 < f < 1.0, got" + std::to_string(f) +
                "."
            );
        }
    #endif
    Log_Debug3(
        "fraxis at " << xy << ", fraction: " << f << ", delta: ("
        << dX << "," << dY << ",c" << c << ")"
    );

    CoordSet changed;

    // Debug
    int count = 0;

    // Calculate next digit
    Coord xyWorking(xy);
    while (
        (locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below) &&
        (abs(f) > m_config.epsilon())
        #ifdef MDN_DEBUG
            && ++count < 100
        #endif
    ) {
        f *= static_cast<double>(m_config.base());
        Digit d(f);
        if (d != 0) {
            changed.merge(locked_add(xyWorking, d));
            changed.merge(internal_fraxisCascade(xyWorking, d, c));
        }
        f -= d;
        xyWorking.translate(dX, dY);
    }
    if (changed.size()) {
        internal_modified();
    }
    return changed;
}


mdn::CoordSet mdn::Mdn2d::internal_fraxisCascade(const Coord& xy, Digit d, int c)
{
    if (Log_Showing_Debug4) {
        Log_Debug4("cascade: " << static_cast<int>(d) << " at " << xy);
    }
    CoordSet changed;
    Coord xyNext = xy.copyTranslate(c, -c);
    d *= -1;
    if (locked_checkPrecisionWindow(xyNext) == PrecisionStatus::Below) {
        // Cascade done
        return changed;
    }
    changed.merge(locked_add(xyNext, d));
    changed.merge(internal_fraxisCascade(xyNext, d, c));
    return changed;
}


mdn::Mdn2d mdn::Mdn2d::internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const {
    Log_Debug4("value: " << value << ", shift: " << shiftXY);
    Mdn2d temp = Duplicate(*this);
    auto tempLock = temp.lockWriteable();
    temp.locked_multiply(value);
    temp.shift(shiftXY);
    return temp;
}
