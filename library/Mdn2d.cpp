#include "Mdn2d.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.hpp"
#include "Logger.hpp"
#include "MdnException.hpp"
#include "Tools.hpp"


mdn::Mdn2d::Mdn2d(std::string nameIn) :
    Mdn2dRules(nameIn)
{
    Log_N_Debug2("");
}


mdn::Mdn2d::Mdn2d(Mdn2dConfig config, std::string nameIn) :
    Mdn2dRules(config, nameIn)
{
    Log_N_Debug2("Config=" << config);
}


mdn::Mdn2d::Mdn2d(const Mdn2d& other, std::string nameIn):
    Mdn2dRules(other, nameIn)
{
    Log_N_Debug2("");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    Mdn2dRules::operator=(other);
    Log_N_Debug2("Copying");
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other, std::string nameIn) noexcept :
    Mdn2dRules(std::move(other), nameIn)
{
    Log_N_Debug2("");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    Mdn2dRules::operator=(std::move(other));
    Log_N_Debug2("Move copying");
    return *this;
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_N_Debug2_H("ans = *this + rhs");
    CoordSet changed = locked_plus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans = *this + rhs");
    CoordSet changed;
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::minus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "minus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_N_Debug2_H("ans = *this - rhs");
    CoordSet changed = locked_minus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans = *this - rhs");
    CoordSet changed;
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, -digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "multiply operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    Log_N_Debug2_H("ans = *this * rhs");
    CoordSet changed = locked_multiply(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans = *this * rhs");
    ans.locked_clear();
    for (const auto& [xy, digit] : rhs.m_raw) {
        int id = static_cast<int>(digit);
        // ans += (this x rhs_id).shift(rhs_xy)
        ans.locked_plusEquals(internal_copyMultiplyAndShift(id, xy));
    }
    Log_N_Debug3_T("ans has " << ans.m_index.size() << " changed digits");
    return ans.m_index;
}


void mdn::Mdn2d::divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    assertNotSelf(ans, "divide operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    }
    CoordSet changed = locked_divide(rhs, ans, fraxis);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    }
    // TODO
    Log_N_Debug3_T("");
    return CoordSet();
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    internal_checkFraxis(fraxis);
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(realNum), fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("");
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_N_Debug2("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, realNum, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, double realNum, Fraxis fraxis) {
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    CoordSet changed = locked_add(xy, static_cast<long>(intPart));
    changed.merge(locked_addFraxis(xy, fracPart, fraxis));
    internal_operationComplete();
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("");
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(-realNum), fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("");
    internal_checkFraxis(fraxis);
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    }
    locked_carryoverCleanup(locked_add(xy, -realNum, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("add " << static_cast<int>(value) << ", no fraxis");
    }
    int ivalue = static_cast<int>(value);
    locked_carryoverCleanup(locked_add(xy, ivalue));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, Digit value) {
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H("add " << static_cast<int>(value) << ", no fraxis");
    }
    CoordSet changed = locked_add(xy, static_cast<int>(value));
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::add(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, int value) {
    int val = static_cast<int>(locked_getValue(xy));
    int sum = val + value;
    int carry = sum / m_config.base();
    int rem = sum % m_config.base();
    CoordSet changed;
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );

    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry));
        changed.merge(locked_add(xy.translatedY(1), carry));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
    // internal_operationComplete();
}


void mdn::Mdn2d::add(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long value) {
    long val = static_cast<long>(locked_getValue(xy));
    long sum = val + value;
    long carry = sum / m_config.base();
    long rem = sum % m_config.base();
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry));
        changed.merge(locked_add(xy.translatedY(1), carry));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::add(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("at " << xy << ", add " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long long value) {
    long long val = static_cast<long long>(locked_getValue(xy));
    long long sum = val + value;
    long long carry = sum / m_config.base();
    long long rem = sum % m_config.base();
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry));
        changed.merge(locked_add(xy.translatedY(1), carry));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("at " << xy << ", add " << static_cast<int>(value) << ", no fraxis");
    }
    locked_carryoverCleanup(locked_add(xy, -static_cast<int>(value)));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    locked_carryoverCleanup(locked_add(xy, -value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::addFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, static_cast<double>(fraction), fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, fraction, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    if (fraction < -1.0 || fraction > 1.0) {
        std::invalid_argument error(
            "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
            "."
        );
        Log_N_Error(error.what());
        throw error;
    }
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    }

    CoordSet changed;
    switch(fraxis) {
        case Fraxis::X:
            changed = internal_fraxis(xy.translatedX(-1), fraction, -1, 0, -1);
            break;
        case Fraxis::Y:
            changed = internal_fraxis(xy.translatedY(-1), fraction, 0, -1, 1);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            std::invalid_argument err(oss.str());
            Log_N_Error(err.what());
            throw err;
            break;
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, -static_cast<double>(fraction), fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    }
    locked_carryoverCleanup(locked_addFraxis(xy, -fraction, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(Digit value) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_N_Debug2_H("scalar multiply " << static_cast<int>(value));
    }
    locked_carryoverCleanup(locked_multiply(static_cast<int>(value)));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(long value) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(long long value) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(int value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        int vxd = value*static_cast<int>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long vxd = value*static_cast<long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long long value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long long vxd = value*static_cast<long long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    operator=(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    auto lock = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    Log_N_Debug2_H("");
    if (rhs.m_config != m_config) {
        Log_N_Debug2_T("equality test, result: config differs");
        return false;
    }
    if (m_raw == rhs.m_raw) {
        Log_N_Debug2_T("equality test, result: data are equal");
        return true;
    }
    if (
        m_config.signConvention() == SignConvention::Positive ||
        m_config.signConvention() == SignConvention::Negative
    ) {
        Log_N_Debug2_T(
            "equality test, result: data !=, with sign convention - no need for polymorphism tests"
        );
        return false;
    }
    Log_N_Debug2("equality test, digits differ, checking polymorphism");
    Mdn2d dlhs = Duplicate(*this);
    Mdn2d drhs = Duplicate(rhs);
    auto dlhsLock = dlhs.lockWriteable();
    auto drhsLock = drhs.lockWriteable();
    dlhs.m_config.setSignConvention(SignConvention::Positive);
    drhs.m_config.setSignConvention(SignConvention::Positive);
    dlhs.locked_carryoverCleanupAll();
    drhs.locked_carryoverCleanupAll();
    bool result = dlhs.m_raw == drhs.m_raw;
    Log_N_Debug2_T(
        "equality test, final polymorphism result = " << result
    );
    return result;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    Log_N_Debug3_H("inequality test");
    bool result = !(*this == rhs);
    Log_N_Debug3_T("result=" << result);
    return result;
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_N_Debug2_H("plus equals");
    CoordSet changed = locked_plusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_plusEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("plus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_N_Debug2_H("minus equals");
    CoordSet changed = locked_minusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_minusEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("minus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, -digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    Log_N_Debug2_H("times equals (Mdn2d)");
    locked_carryoverCleanup(locked_timesEquals(rhs));
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_timesEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("times equals (Mdn2d)");
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_multiply(rhs, temp);
    operator=(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    Log_N_Debug3_H("divide equals (Mdn2d)");
    Mdn2d temp = Duplicate(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_divide(rhs, temp);
    operator=(temp);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    auto lock = lockWriteable();
    Log_N_Debug3_H("times equals (int scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long scalar) {
    auto lock = lockWriteable();
    Log_N_Debug3_H("times equals (long scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long long scalar) {
    auto lock = lockWriteable();
    Log_N_Debug3_H("times equals (long long scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


void mdn::Mdn2d::internal_checkFraxis(Fraxis& fraxis) const {
    if (fraxis == Fraxis::Default) {
        if (Log_Showing_Debug4) {
            Log_N_Debug4("applying default fraxis: " << FraxisToName(fraxis));
        }
        fraxis = m_config.fraxis();
    } else {
        if (Log_Showing_Debug4) {
            Log_N_Debug4("supplied fraxis: " << FraxisToName(fraxis));
        }
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
    Log_N_Debug3_H(
        "fraxis at " << xy << ", fraction: " << f << ", delta: ("
        << dX << "," << dY << ",c" << c << ")"
    );

    CoordSet changed;

    int count = 0;

    // Calculate next digit
    Coord xyWorking(xy);
    bool precisionOkay = locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below;
    bool numericsOkay = fabs(f) > m_config.epsilon();
    if (Log_Showing_Debug4) {
        Log_N_Debug4(
            "internal_fraxis loop, starting at " << xyWorking << ", "
            << "current vals: f=" << f << ", " << "xy=" << xyWorking << ", "
            << "loop: prec=" << precisionOkay << ", num=" << numericsOkay
        );
    }
    while (precisionOkay && numericsOkay && ++count < 1000) {
        f *= m_config.baseDouble();
        Digit d(f);
        if (d != 0) {
            changed.merge(locked_add(xyWorking, d));
            changed.merge(internal_fraxisCascade(xyWorking, d, c));
        }
        f -= d;
        xyWorking.translate(dX, dY);
        precisionOkay = locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below;
        numericsOkay = fabs(f) > m_config.epsilon();
        if (Log_Showing_Debug4) {
            Log_N_Debug4(
                "internal_fraxis loop, added " << static_cast<int>(d) << " to "
                << xyWorking.translated(-dX, -dY) << ", "
                << "current vals: f=" << f << ", " << "xy=" << xyWorking << ", "
                << "loop: prec=" << precisionOkay << ", num=" << numericsOkay
            );
        }
    }
    if (precisionOkay && numericsOkay) {
        Log_N_Warn(
            "Internal error: possible infinite loop detected in internal_fraxis algorithm.\n"
            << "parameters: fraxis at " << xy << ", fraction: " << f << ", delta: ("
            << dX << "," << dY << ",c" << c << ")"
        );
    }

    if (changed.size()) {
        internal_modified();
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::internal_fraxisCascade(const Coord& xy, Digit d, int c)
{
    if (Log_Showing_Debug4) {
        Log_N_Debug4_H("cascade: " << static_cast<int>(d) << " at " << xy);
    }
    CoordSet changed;
    Coord xyNext = xy.translated(c, -c);
    d *= -1;
    if (locked_checkPrecisionWindow(xyNext) == PrecisionStatus::Below) {
        // Cascade done
        Log_N_Debug3_T(
            "cascaded to edge of precision window, changed " << changed.size() << " digits"
        );
        return changed;
    }
    changed.merge(locked_add(xyNext, d));
    changed.merge(internal_fraxisCascade(xyNext, d, c));
    Log_N_Debug3_T("finalising cascade, changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d mdn::Mdn2d::internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const {
    Log_N_Debug4_H("value: " << value << ", shift: " << shiftXY);
    Mdn2d temp = Duplicate(*this);
    auto tempLock = temp.lockWriteable();
    temp.locked_multiply(value);
    temp.shift(shiftXY);
    Log_N_Debug4_T("");
    return temp;
}
