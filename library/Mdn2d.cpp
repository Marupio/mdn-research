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
{}


mdn::Mdn2d::Mdn2d(Mdn2dConfig config) :
    Mdn2dRules(config)
{}


mdn::Mdn2d::Mdn2d(const Mdn2d& other):
    Mdn2dRules(other)
{}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    Mdn2dRules::operator=(other);
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other) noexcept :
    Mdn2dRules(std::move(other))
{}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    Mdn2dRules::operator=(std::move(other));
    return *this;
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    CoordSet changed = locked_plus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
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
    CoordSet changed = locked_minus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
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
    CoordSet changed = locked_multiply(rhs, ans);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
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
    CoordSet changed = locked_divide(rhs, ans, fraxis);
    ans.locked_carryoverCleanup(changed);
}


mdn::CoordSet mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    // TODO
    return CoordSet();
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, Fraxis fraxis) {
    internal_checkFraxis(fraxis);
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(realNum), fraxis));
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, realNum, fraxis));
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, double realNum, Fraxis fraxis) {
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    modified();
    CoordSet changed = locked_add(xy, static_cast<long>(intPart));
    changed.merge(locked_addFraxis(xy, fracPart, fraxis));
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, static_cast<double>(-realNum), fraxis));
}


void mdn::Mdn2d::subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, -realNum, fraxis));
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    int ivalue = static_cast<int>(value);
    modified();
    locked_carryoverCleanup(locked_add(xy, ivalue));
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, Digit value) {
    return locked_add(xy, static_cast<int>(value));
}


void mdn::Mdn2d::add(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, value));
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, int value) {
    int val = static_cast<int>(locked_getValue(xy));
    int sum = val + value;
    int carry = sum / m_config.base();
    int rem = sum % m_config.base();
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


void mdn::Mdn2d::add(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, value));
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long value) {
    long val = static_cast<long>(locked_getValue(xy));
    long sum = val + value;
    long carry = sum / m_config.base();
    long rem = sum % m_config.base();
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
    modified();
    locked_carryoverCleanup(locked_add(xy, value));
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long long value) {
    long long val = static_cast<long long>(locked_getValue(xy));
    long long sum = val + value;
    long long carry = sum / m_config.base();
    long long rem = sum % m_config.base();
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
    modified();
    locked_carryoverCleanup(locked_add(xy, -static_cast<int>(value)));
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, -value));
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, -value));
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_add(xy, -value));
}


void mdn::Mdn2d::addFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_addFraxis(xy, static_cast<double>(fraction), fraxis));
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_addFraxis(xy, fraction, fraxis));
}


mdn::CoordSet mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    if (fraction < -1.0 || fraction > 1.0) {
        throw std::invalid_argument(
            "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
            "."
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
            throw std::invalid_argument(oss.str());
            break;
    }
    return changed;
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_addFraxis(xy, -static_cast<double>(fraction), fraxis));
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_addFraxis(xy, -fraction, fraxis));
}


void mdn::Mdn2d::multiply(Digit value) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_multiply(static_cast<int>(value)));
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_multiply(value));
}


void mdn::Mdn2d::multiply(long value) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_multiply(value));
}


void mdn::Mdn2d::multiply(long long value) {
    auto lock = lockWriteable();
    modified();
    locked_carryoverCleanup(locked_multiply(value));
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(int value) {
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
        return false;
    }
    if (m_raw == rhs.m_raw) {
        return true;
    }
    if (
        m_config.signConvention() == SignConvention::Positive ||
        m_config.signConvention() == SignConvention::Negative
    ) {
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
    return dlhs.m_raw == drhs.m_raw;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    return !(*this == rhs);
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    CoordSet changed = locked_plusEquals(rhs);
    locked_carryoverCleanup(changed);
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_plusEquals(const Mdn2d& rhs) {
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, digit));
    }
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    CoordSet changed = locked_minusEquals(rhs);
    locked_carryoverCleanup(changed);
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_minusEquals(const Mdn2d& rhs) {
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, -digit));
    }
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    locked_carryoverCleanup(locked_timesEquals(rhs));
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_timesEquals(const Mdn2d& rhs) {
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_multiply(rhs, temp);
    operator=(temp);
    modified();
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    Mdn2d temp = Duplicate(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_divide(rhs, temp);
    operator=(temp);
    modified();
    locked_carryoverCleanup(changed);
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(scalar));
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long scalar) {
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(scalar));
    modified();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long long scalar) {
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(scalar));
    modified();
    return *this;
}


void mdn::Mdn2d::internal_checkFraxis(Fraxis& fraxis) const {
    if (fraxis == Fraxis::Default) {
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
    return changed;
}


mdn::CoordSet mdn::Mdn2d::internal_fraxisCascade(const Coord& xy, Digit d, int c)
{
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
    Mdn2d temp = Duplicate(*this);
    auto tempLock = temp.lockWriteable();
    temp.locked_multiply(value);
    temp.shift(shiftXY);
    return temp;
}
