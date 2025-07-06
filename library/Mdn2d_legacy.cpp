#include "Mdn2d.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


double mdn::Mdn2d::static_calculateEpsilon(int m_precision, int m_base) {
    return pow((1.0 / m_base), (m_precision + 1));
}


mdn::Carryover mdn::Mdn2d::static_checkCarryover(Digit p, Digit x, Digit y, Digit base) {
    if (abs(p) > base) {
        return Carryover::Required;
    }

    auto pos = [](Digit t) { return t > 0; };
    auto neg = [](Digit t) { return t < 0; };

    if (pos(p)) {
        if (neg(x) && neg(y)) {
            return Carryover::Required;
        }
        else if (neg(x) || neg(y)) {
            return Carryover::Optional;
        }
    }
    else if (neg(p)) {
        if (pos(x) && pos(y)) {
            return Carryover::Required;
        }
        else if (pos(x) || pos(y)) {
            return Carryover::Optional;
        }
    }
    return Carryover::Invalid;
}


mdn::Mdn2d::Mdn2d(int base, int precision)
:
    m_base(base),
    m_dbase(m_base),
    m_precision(precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_polymorphicNodes_event(-1),
    m_event(0)
{}


mdn::Mdn2d::Mdn2d(int base, int precision, int initVal)
:
    m_base(base),
    m_dbase(m_base),
    m_precision(precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_polymorphicNodes_event(-1),
    m_event(0)
{
    locked_add(Coord({0, 0}), initVal);
}


mdn::Mdn2d::Mdn2d(int base, int precision, double initVal, Fraxis fraxis)
:
    m_base(base),
    m_dbase(m_base),
    m_precision(precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_polymorphicNodes_event(-1),
    m_event(0)
{
    locked_add(COORD_ORIGIN, initVal, fraxis);
}


mdn::Mdn2d::Mdn2d(const Mdn2d& other):
    m_base(other.m_base),
    m_dbase(other.m_dbase),
    m_precision(other.m_precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_polymorphicNodes_event(-1),
    m_event(0)
{
    auto lock = other.lockReadOnly();

    m_raw = other.m_raw;
    m_xIndex = other.m_xIndex;
    m_yIndex = other.m_yIndex;
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
    if (other.m_event == other.m_polymorphicNodes_event) {
        m_polymorphicNodes = other.m_polymorphicNodes;
        m_polymorphicNodes_event = m_event;
    }
}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();

        if (m_base != other.m_base) {
            throw BaseMismatch(m_base, other.m_base);
        }
        m_precision = other.m_precision;
        m_epsilon = other.m_epsilon;
        m_raw = other.m_raw;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        modified();
        if (other.m_event == other.m_polymorphicNodes_event) {
            m_polymorphicNodes = other.m_polymorphicNodes;
            m_polymorphicNodes_event = m_event;
        }
    }
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other) noexcept :
    m_base(other.m_base),
    m_dbase(m_base),
    m_precision(other.m_precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_event(0)
{
    auto lock = other.lockWriteable();

    m_raw = std::move(other.m_raw);
    m_xIndex = std::move(other.m_xIndex);
    m_yIndex = std::move(other.m_yIndex);
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
    if (other.m_event == other.m_polymorphicNodes_event) {
        m_polymorphicNodes = other.m_polymorphicNodes;
        m_polymorphicNodes_event = m_event;
    }
}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockWriteable();

        if (m_base != other.m_base) {
            throw BaseMismatch(m_base, other.m_base);
        }
        m_precision = other.m_precision;
        m_epsilon = other.m_epsilon;
        m_raw = std::move(other.m_raw);
        m_xIndex = std::move(other.m_xIndex);
        m_yIndex = std::move(other.m_yIndex);
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        if (other.m_event == other.m_polymorphicNodes_event) {
            m_polymorphicNodes = std::move(other.m_polymorphicNodes);
            m_polymorphicNodes_event = m_event;
        }
        modified();
    }
    return *this;
}


mdn::Digit mdn::Mdn2d::getValue(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_getValue(xy);
}


mdn::Digit mdn::Mdn2d::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        return static_cast<Digit>(0);
    }
    return it->second;
}


std::vector<mdn::Digit> mdn::Mdn2d::getRow(int y) const {
    auto lock = lockReadOnly();
    return locked_getRow(y);
}


std::vector<mdn::Digit> mdn::Mdn2d::locked_getRow(int y) const {
    std::vector<Digit> digits;
    locked_getRow(y, digits);
}


void mdn::Mdn2d::getRow(int y, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_getRow(y, digits);
}


void mdn::Mdn2d::locked_getRow(int y, std::vector<Digit>& digits) const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x() + 1; // one past the end, prevent fencepost
    int xCount = xEnd - xStart;
    digits.resize(xCount);
    std::fill(digits.begin(), digits.end(), 0);
    auto it = m_yIndex.find(y);
    if (it != m_yIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            digits[coord.x()-xStart] = m_raw.at(coord);
        }
    }
}


std::vector<mdn::Digit> mdn::Mdn2d::getCol(int x) const {
    auto lock = lockReadOnly();
    return locked_getCol(x);
}


std::vector<mdn::Digit> mdn::Mdn2d::locked_getCol(int x) const {
    std::vector<Digit> digits;
    locked_getCol(x, digits);
}


void mdn::Mdn2d::getCol(int x, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_getCol(x, digits);
}


void mdn::Mdn2d::locked_getCol(int x, std::vector<Digit>& digits) const {
    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y() + 1; // one past the end, prevent fencepost
    int yCount = yEnd - yStart;
    digits.resize(yCount);
    std::fill(digits.begin(), digits.end(), 0);
    auto it = m_xIndex.find(x);
    if (it != m_xIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            digits[coord.x()-yStart] = m_raw.at(coord);
        }
    }
}


void mdn::Mdn2d::clear() {
    auto lock = lockWriteable();
    locked_clear();
    modified();
}


void mdn::Mdn2d::locked_clear() {
    m_raw.clear();
    internal_clearMetadata();
}


bool mdn::Mdn2d::setToZero(const Coord& xy) {
    auto lock = lockWriteable();
    modified();
    return locked_setToZero(xy);
}


bool mdn::Mdn2d::locked_setToZero(const Coord& xy) {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // Already zero
        return locked_checkPrecisionWindow(xy) != PrecisionStatus::Below;
    }
    // There is currently a non-zero value - erase it
    auto xit(m_xIndex.find(xy.x()));
    auto yit(m_yIndex.find(xy.y()));
    #ifdef MDN_DEBUG
        // Debug mode - sanity check - metadata entries must be non-zero
        if (xit == m_xIndex.end() || yit == m_yIndex.end()) {
            std::ostringstream oss;
            oss << "Internal error: addressing data invalid, discovered when zeroing ";
            oss << "coord: " << xy << std::endl;
            oss << "Rebuilding metadata." << std::endl;
            Logger::instance().warn(oss.str());
            rebuildMetadata();
            xit = m_xIndex.find(xy.x());
            yit = m_yIndex.find(xy.y());
        }
    #endif // MDN_DEBUG
    m_raw.erase(it);

    std::unordered_set<Coord>& coordsAlongX(xit->second);
    std::unordered_set<Coord>& coordsAlongY(yit->second);
    coordsAlongX.erase(xy);
    coordsAlongY.erase(xy);
    bool checkBounds = false;
    if (coordsAlongX.size() == 0) {
        m_xIndex.erase(xit);
        checkBounds = true;
    }

    if (coordsAlongY.size() == 0) {
        m_yIndex.erase(yit);
        checkBounds = true;
    }
    if (checkBounds) {
        // Bounds may have changed
        internal_updateBounds();
    }
    return true;
}


int mdn::Mdn2d::setToZero(const std::unordered_set<Coord>& coords) {
    auto lock = lockWriteable();
    modified();
    return locked_setToZero(coords);
}


int mdn::Mdn2d::locked_setToZero(const std::unordered_set<Coord>& purgeSet) {
    int nZeroed = 0;

    // Step 1: Erase from m_raw
    for (const Coord& coord : purgeSet) {
        nZeroed += m_raw.erase(coord);
    }

    // Step 2: Clean up m_xIndex and m_yIndex
    for (const Coord& coord : purgeSet) {
        int x = coord.x();
        int y = coord.y();

        // Erase coord from x index
        auto xIt = m_xIndex.find(x);
        if (xIt != m_xIndex.end()) {
            xIt->second.erase(coord);
            if (xIt->second.empty()) {
                m_xIndex.erase(xIt);
            }
        }

        // Erase coord from y index
        auto yIt = m_yIndex.find(y);
        if (yIt != m_yIndex.end()) {
            yIt->second.erase(coord);
            if (yIt->second.empty()) {
                m_yIndex.erase(yIt);
            }
        }
    }
    // Bounds may have changed
    internal_updateBounds();
    return nZeroed;
}


bool mdn::Mdn2d::setValue(const Coord& xy, Digit value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, value);
}


bool mdn::Mdn2d::setValue(const Coord& xy, int value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2d::setValue(const Coord& xy, long value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2d::setValue(const Coord& xy, long long value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2d::locked_setValue(const Coord& xy, Digit value) {
    if (value == 0) {
        return locked_setToZero(xy);
    }
    internal_checkDigit(xy, value);

    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // No entry exists
        PrecisionStatus ps = locked_checkPrecisionWindow(xy);
        if (ps == PrecisionStatus::Below) {
            // Out of numerical precision range
            return false;
        }
        internal_insertAddress(xy);
        m_raw[xy] = value;
        if (ps == PrecisionStatus::Above) {
            internal_purgeExcessDigits();
        }
        return true;
    }
    // xy is already non-zero
    it->second = value;
    return true;
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    locked_plus(rhs, ans);
}


void mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        ans.locked_add(xy, digit);
    }
}


void mdn::Mdn2d::minus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "minus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    locked_minus(rhs, ans);
}


void mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
    ans = *this;
    for (const auto& [xy, digit] : rhs.m_raw) {
        ans.locked_add(xy, -digit);
    }
}


void mdn::Mdn2d::multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "multiply operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    locked_multiply(rhs, ans);
}


void mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    ans.locked_clear();
    for (const auto& [xy, digit] : rhs.m_raw) {
        int id = static_cast<int>(digit);
        // ans += (this x rhs_id).shift(rhs_xy)
        ans.locked_plusEquals(internal_copyMultiplyAndShift(id, xy));
    }
}


void mdn::Mdn2d::divide(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "divide operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    locked_divide(rhs, ans);
}


void mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans) const {

    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, Fraxis fraxis) {
    internal_checkFraxis(fraxis);
    auto lock = lockWriteable();
    modified();
    locked_add(xy, static_cast<double>(realNum), fraxis);
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, realNum, fraxis);
}


void mdn::Mdn2d::locked_add(const Coord& xy, double realNum, Fraxis fraxis) {
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    modified();
    locked_add(xy, static_cast<long>(intPart));
    locked_addFraxis(xy, fracPart, fraxis);
}


void mdn::Mdn2d::subtract(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, static_cast<double>(-realNum), fraxis);
}


void mdn::Mdn2d::subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, -realNum, fraxis);
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    int ivalue = static_cast<int>(value);
    modified();
    locked_add(xy, ivalue);
}


void mdn::Mdn2d::add(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, int value) {
    int val = static_cast<int>(locked_getValue(xy));
    int sum = val + value;
    int carry = sum / m_base;
    int rem = sum % m_base;
    locked_setValue(xy, rem);
    if (carry != 0) {
        locked_add(xy.copyTranslateX(1), carry);
        locked_add(xy.copyTranslateY(1), carry);
    }
}


void mdn::Mdn2d::add(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, long value) {
    long val = static_cast<long>(locked_getValue(xy));
    long sum = val + value;
    long carry = sum / m_base;
    long rem = sum % m_base;
    locked_setValue(xy, rem);
    if (carry != 0) {
        locked_add(xy.copyTranslateX(1), carry);
        locked_add(xy.copyTranslateY(1), carry);
    }
}


void mdn::Mdn2d::add(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, long long value) {
    long long val = static_cast<long long>(locked_getValue(xy));
    long long sum = val + value;
    long long carry = sum / m_base;
    long long rem = sum % m_base;
    locked_setValue(xy, rem);
    if (carry != 0) {
        locked_add(xy.copyTranslateX(1), carry);
        locked_add(xy.copyTranslateY(1), carry);
    }
}


void mdn::Mdn2d::subtract(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, -static_cast<int>(value));
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, -value);
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, -value);
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    modified();
    locked_add(xy, -value);
}


void mdn::Mdn2d::addFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_addFraxis(xy, static_cast<double>(fraction), fraxis);
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_addFraxis(xy, fraction, fraxis);
}


void mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    if (fraction < -1.0 || fraction > 1.0) {
        throw std::invalid_argument(
            "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
            "."
        );
    }

    switch(fraxis) {
        case Fraxis::X:
            internal_fraxis(xy.copyTranslateX(-1), fraction, -1, 0, -1);
            break;
        case Fraxis::Y:
            internal_fraxis(xy.copyTranslateY(-1), fraction, 0, -1, 1);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            throw std::invalid_argument(oss.str());
            break;
    }
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_addFraxis(xy, -static_cast<double>(fraction), fraxis);
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    modified();
    locked_addFraxis(xy, -fraction, fraxis);
}


void mdn::Mdn2d::multiply(Digit value) {
    auto lock = lockWriteable();
    modified();
    locked_multiply(static_cast<int>(value));
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    modified();
    locked_multiply(value);
}


void mdn::Mdn2d::multiply(long value) {
    auto lock = lockWriteable();
    modified();
    locked_multiply(value);
}


void mdn::Mdn2d::multiply(long long value) {
    auto lock = lockWriteable();
    modified();
    locked_multiply(value);
}


void mdn::Mdn2d::locked_multiply(int value) {
    Mdn2d temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();

    for (const auto& [xy, digit] : m_raw) {
        int vxd = value*static_cast<int>(digit);
        temp.locked_add(xy, vxd);
    }
    operator=(temp);
}


void mdn::Mdn2d::locked_multiply(long value) {
    Mdn2d temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();

    for (const auto& [xy, digit] : m_raw) {
        long vxd = value*static_cast<long>(digit);
        temp.locked_add(xy, vxd);
    }
    operator=(temp);
}


void mdn::Mdn2d::locked_multiply(long long value) {
    Mdn2d temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();

    for (const auto& [xy, digit] : m_raw) {
        long long vxd = value*static_cast<long long>(digit);
        temp.locked_add(xy, vxd);
    }
    operator=(temp);
}


std::string mdn::Mdn2d::toString() const {
    auto lock = lockReadOnly();
    return locked_toString();
}


std::string mdn::Mdn2d::locked_toString() const {
    std::vector<std::string> rows = locked_toStringRows();
    return Tools::joinArray(rows, "\n", true);
}


std::vector<std::string> mdn::Mdn2d::toStringRows() const {
    auto lock = lockReadOnly();
    return locked_toStringRows();
}


std::vector<std::string> mdn::Mdn2d::locked_toStringRows() const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x()+1;
    int xCount = xEnd - xStart;

    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y()+1;
    int yCount = yEnd - yStart;

    // DigLine - digit line appears before what index in 'digits' array below
    int xDigLine = -xStart;
    int yDigLine = 0;

    std::vector<std::string> out;
    out.reserve(yCount+1);
    std::vector<Digit> digits;
    for (int y = yStart; y < yEnd; ++y) {
        // First, are we at the yDigit line?
        if (y == yDigLine) {
            int x;
            std::ostringstream oss;
            for (x = 0; x < xDigLine && x < xCount; ++x) {
                oss << Tools::m_boxArt_h << Tools::m_boxArt_h;
            }
            if (x == xDigLine) {
                oss << Tools::m_boxArt_x;
            }
            for (; x < xCount; ++x) {
                oss << Tools::m_boxArt_h << Tools::m_boxArt_h;
            }
            out.push_back(oss.str());
        }
        locked_getRow(y, digits);
        assert(digits.size() == yCount && "Digits not the correct size");
        // std::string rowStr;
        std::ostringstream oss;
        int x;
        for (x = 0; x < xDigLine && x < xCount; ++x) {
            oss << Tools::digitToAlpha(digits[x]);
        }
        if (x == xDigLine) {
            oss << Tools::m_boxArt_v;
        }
        for (; x < xCount; ++x) {
            oss << Tools::digitToAlpha(digits[x]);
        }
        out.push_back(oss.str());
    } // end y loop
    return out;
}


std::vector<std::string> mdn::Mdn2d::toStringCols() const {
    auto lock = lockReadOnly();
    return locked_toStringCols();
}


std::vector<std::string> mdn::Mdn2d::locked_toStringCols() const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x()+1;
    int xCount = xEnd - xStart;

    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y()+1;
    int yCount = yEnd - yStart;

    // DigLine - digit line appears before what index in 'digits' array below
    int xDigLine = 0;
    int yDigLine = -yStart;

    std::vector<std::string> out;
    out.reserve(xCount+1);
    std::vector<Digit> digits;
    for (int x = xStart; x < xEnd; ++y) {
        // First, are we at the yDigit line?
        if (x == xDigLine) {
            int y;
            std::ostringstream oss;
            for (y = 0; y < yDigLine && y < yCount; ++y) {
                oss << Tools::m_boxArt_h << Tools::m_boxArt_h;
            }
            if (y == yDigLine) {
                oss << Tools::m_boxArt_x;
            }
            for (; y < yCount; ++y) {
                oss << Tools::m_boxArt_h << Tools::m_boxArt_h;
            }
            out.push_back(oss.str());
        }
        locked_getCol(x, digits);
        assert(digits.size() == xCount && "Digits not the correct size");
        std::ostringstream oss;
        int y;
        for (y = 0; y < yDigLine && y < yCount; ++y) {
            oss << Tools::digitToAlpha(digits[y]);
        }
        if (y == yDigLine) {
            oss << Tools::m_boxArt_v;
        }
        for (; y < yCount; ++y) {
            oss << Tools::digitToAlpha(digits[y]);
        }
        out.push_back(oss.str());
    } // end y loop
    return out;
}


mdn::Carryover mdn::Mdn2d::checkCarryover(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_checkCarryover(xy);
}


mdn::Carryover mdn::Mdn2d::locked_checkCarryover(const Coord& xy) const {
    return static_checkCarryover(
        locked_getValue(xy),
        locked_getValue(xy.copyTranslateX(1)),
        locked_getValue(xy.copyTranslateY(1)),
        m_dbase
    );
}


void mdn::Mdn2d::carryover(const Coord& xy)
{
    auto lock = lockWriteable();
    modified();
    locked_carryover(xy);
}


void mdn::Mdn2d::locked_carryover(const Coord& xy) {
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    Carryover co = static_checkCarryover(p, x, y, m_dbase);
    if (co == Carryover::Invalid) {
        throw IllegalOperation("Invalid carryover requested at " + xy.to_string());
    }
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_base;
    ip -= nCarry * m_base;
    iy += nCarry;
    ix += nCarry;

    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
}


void mdn::Mdn2d::shift(int xDigits, int yDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shift(xDigits, yDigits);
}


void mdn::Mdn2d::shift(const Coord& xy) {
    auto lock = lockWriteable();
    modified();
    locked_shift(xy);
}


void mdn::Mdn2d::locked_shift(const Coord& xy) {
    locked_shift(xy.x(), xy.y());
}


void mdn::Mdn2d::locked_shift(int xDigits, int yDigits) {
    if (xDigits > 0) {
        locked_shiftRight(xDigits);
    } else if (xDigits < 0) {
        locked_shiftLeft(-xDigits);
    }
    if (yDigits > 0) {
        locked_shiftUp(yDigits);
    } else if (yDigits < 0) {
        locked_shiftDown(-yDigits);
    }
}


void mdn::Mdn2d::shiftRight(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftRight(nDigits);
}


void mdn::Mdn2d::locked_shiftRight(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_xIndex.rbegin(); it != m_xIndex.rend(); ++it) {
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateX(nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2d::shiftLeft(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftLeft(nDigits);
}


void mdn::Mdn2d::locked_shiftLeft(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_xIndex.begin(); it != m_xIndex.end(); ++it) {
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateX(-nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2d::shiftUp(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftUp(nDigits);
}


void mdn::Mdn2d::locked_shiftUp(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_yIndex.rbegin(); it != m_yIndex.rend(); ++it) {
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateY(nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2d::shiftDown(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftDown(nDigits);
}


void mdn::Mdn2d::locked_shiftDown(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_yIndex.begin(); it != m_yIndex.end(); ++it) {
        const std::unordered_set<Coord>& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateY(-nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2d::transpose() {
    auto lock = lockWriteable();
    modified();
    locked_transpose();
}


void mdn::Mdn2d::locked_transpose() {
    Mdn2d temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();
    for (const auto& [xy, digit] : m_raw) {
        temp.locked_setValue(Coord(xy.y(), xy.x()), digit);
    }
    operator=(temp);
}


void mdn::Mdn2d::rebuildMetadata() const {
    auto lock = lockWriteable();
    locked_rebuildMetadata();
}


void mdn::Mdn2d::locked_rebuildMetadata() const {
    internal_clearMetadata();

    for (const auto& [xy, digit] : m_raw) {
        if (digit == 0) {
            throw ZeroEncountered(xy);
        }
        const int x = xy.x();
        const int y = xy.y();
        internal_insertAddress(xy);
    }

    // This updates bounds based on metadata
    // auto itMinX = m_xIndex.cbegin();
    // auto itMaxX = m_xIndex.crbegin();
    // auto itMinY = m_yIndex.cbegin();
    // auto itMaxY = m_yIndex.crbegin();
    // m_boundsMin = {itMinX->first, itMinY->first};
    // m_boundsMax = {itMaxX->first, itMaxY->first};
}


bool mdn::Mdn2d::hasBounds() const {
    auto lock = lockReadOnly();
    return locked_hasBounds();
}


bool mdn::Mdn2d::locked_hasBounds() const {
    bool invalid = (
        m_boundsMin.x() == constants::intMax ||
        m_boundsMin.y() == constants::intMax ||
        m_boundsMax.x() == constants::intMin ||
        m_boundsMax.y() == constants::intMin
    );

    return !invalid;
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2d::getBounds() const {
    auto lock = ReadOnlyLock();
    return locked_getBounds();
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2d::locked_getBounds() const {
    return std::pair<Coord, Coord>(m_boundsMin, m_boundsMax);
}


const std::unordered_set<mdn::Coord>& mdn::Mdn2d::getPolymorphicNodes() const {
    auto lock = ReadOnlyLock();
    return locked_getPolymorphicNodes();
}


const std::unordered_set<mdn::Coord>& mdn::Mdn2d::locked_getPolymorphicNodes() const {
    if (m_polymorphicNodes_event != m_event) {
        internal_polymorphicScan();
    }
    return m_polymorphicNodes;
}


void mdn::Mdn2d::polymorphism_x0() {
    auto lock = lockWriteable();
    locked_polymorphism_x0();
}


void mdn::Mdn2d::locked_polymorphism_x0() {
    const std::unordered_set<Coord>& pn = locked_getPolymorphicNodes();
    for (const Coord& xy : pn) {
        Digit p = locked_getValue(xy);
        if (p > 0) {
            internal_oneCarryover(xy);
        }
    }
}


void mdn::Mdn2d::polymorphism_y0() {
    auto lock = lockWriteable();
    locked_polymorphism_y0();
}


void mdn::Mdn2d::locked_polymorphism_y0() {
    const std::unordered_set<Coord>& pn = locked_getPolymorphicNodes();
    for (const Coord& xy : pn) {
        Digit p = locked_getValue(xy);
        if (p < 0) {
            internal_oneCarryover(xy);
        }
    }
}


int mdn::Mdn2d::getPrecision() const {
    auto lock = lockReadOnly();
    return locked_getPrecision();
}


int mdn::Mdn2d::locked_getPrecision() const {
    return m_precision;
}


int mdn::Mdn2d::setPrecision(int newMaxSpan) {
    auto lock = lockWriteable();
    modified();
    return locked_setPrecision(newMaxSpan);
}


int mdn::Mdn2d::locked_setPrecision(int newMaxSpan) {
    if (newMaxSpan < 1) {
        throw std::invalid_argument("precision, Mdn2d numerical precision, cannot be less than 1");
    }
    else if (newMaxSpan >= m_precision) {
        m_precision = newMaxSpan;
        m_epsilon = static_calculateEpsilon(m_precision, m_base);
        return;
    }

    // New precision is *less* than the old one - may not be able to keep all our digits
    int oldMaxSpan = m_precision;
    m_precision = newMaxSpan;
    m_epsilon = static_calculateEpsilon(m_precision, m_base);

    return internal_purgeExcessDigits();
}


mdn::PrecisionStatus mdn::Mdn2d::checkPrecisionWindow(const Coord& xy) const {
    auto lock = ReadOnlyLock();
    return locked_checkPrecisionWindow(xy);
}


mdn::PrecisionStatus mdn::Mdn2d::locked_checkPrecisionWindow(const Coord& xy) const {
    if (!locked_hasBounds()) {
        return PrecisionStatus::Inside;
    }
    // minLimit - below this and the new value should not be added
    Coord minLimit = m_boundsMax - m_precision;

    // Check under limit
    if (xy.x() < minLimit.x() || xy.y() < minLimit.y()) {
        // Value is under-limit, do not add
        return PrecisionStatus::Below;
    }

    // Check over limit
    std::unordered_set<Coord> purgeSet;
    // maxLimit - above this and we need to purge existing values
    Coord maxLimit = m_boundsMin + m_precision;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    int purgeY = xy.y() - maxLimit.y();
    if (purgeX > 0 || purgeY > 0) {
        return PrecisionStatus::Above;
    }
    return PrecisionStatus::Inside;
}


void mdn::Mdn2d::modified(){
    m_event++;
}


// *** Operators ***

mdn::Digit mdn::Mdn2d::operator()(int x, int y) const {
    auto lock = lockReadOnly();
    return locked_getValue(Coord(x, y));
}


mdn::Digit mdn::Mdn2d::operator()(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_getValue(xy);
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    return locked_plusEquals(rhs);
}


mdn::Mdn2d& mdn::Mdn2d::locked_plusEquals(const Mdn2d& rhs) {
    for (const auto& [xy, digit] : rhs.m_raw) {
        locked_add(xy, digit);
    }
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    return locked_minusEquals(rhs);
}


mdn::Mdn2d& mdn::Mdn2d::locked_minusEquals(const Mdn2d& rhs) {
    for (const auto& [xy, digit] : rhs.m_raw) {
        locked_add(xy, -digit);
    }
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    auto lock = lockWriteable();
    auto lockRhs = rhs.lockReadOnly();
    modified();
    return locked_timesEquals(rhs);
}


mdn::Mdn2d& mdn::Mdn2d::locked_timesEquals(const Mdn2d& rhs) {
    Mdn2d temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();
    locked_multiply(rhs, temp);
    operator=(temp);
    modified();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    Mdn2d temp(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    locked_divide(rhs, temp);
    operator=(temp);
    modified();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    auto lock = lockWriteable();
    locked_multiply(scalar);
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long scalar) {
    auto lock = lockWriteable();
    locked_multiply(scalar);
    modified();
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long long scalar) {
    auto lock = lockWriteable();
    locked_multiply(scalar);
    modified();
    return *this;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    auto lock = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    Mdn2d lhsCopy(*this);
    Mdn2d rhsCopy(rhs);
    lhsCopy.locked_polymorphism_x0();
    rhsCopy.locked_polymorphism_x0();
    return lhsCopy.m_raw == rhsCopy.m_raw;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    return !(*this == rhs);
}


// *** Private functions ***

mdn::Mdn2d::WritableLock mdn::Mdn2d::lockWriteable() const {
    return std::unique_lock(m_mutex);
}


mdn::Mdn2d::ReadOnlyLock mdn::Mdn2d::lockReadOnly() const {
    return std::shared_lock(m_mutex);
}


void mdn::Mdn2d::assertNotSelf(Mdn2d& that, const std::string& description) const {
    if (this == &that) {
        throw IllegalSelfReference(description);
    }
}


// mdn::Digit& mdn::Mdn2d::operator()(int x, int y) {
//     return operator()(Coord(x, y));
// }
//
//
// mdn::Digit& mdn::Mdn2d::operator()(const Coord& xy) {
//     return m_raw[xy];
// }


void mdn::Mdn2d::internal_clearMetadata() const {
    m_boundsMin = Coord({constants::intMax, constants::intMax});
    m_boundsMax = Coord({constants::intMin, constants::intMin});

    m_xIndex.clear();
    m_yIndex.clear();

    m_polymorphicNodes.clear();
}


void mdn::Mdn2d::internal_insertAddress(const Coord& xy) const {
    auto xit = m_xIndex.find(xy.x());
    if (xit == m_xIndex.end()) {
        m_xIndex.emplace(xy.x(), std::unordered_set<Coord>());
        std::unordered_set<Coord> newX;
        newX.insert(xy);
        m_xIndex[xy.x()] = newX;
    } else {
        xit->second.insert(xy);
    }
    auto yit = m_yIndex.find(xy.y());
    if (yit == m_yIndex.end()) {
        m_yIndex.emplace(xy.y(), std::unordered_set<Coord>());
        std::unordered_set<Coord> newy;
        newy.insert(xy);
        m_yIndex[xy.y()] = newy;
    } else {
        yit->second.insert(xy);
    }
    if (xy.x() < m_boundsMin.x())
        m_boundsMin.x() = xy.x();
    if (xy.x() > m_boundsMax.x())
        m_boundsMax.x() = xy.x();
    if (xy.y() < m_boundsMin.y())
        m_boundsMin.y() = xy.y();
    if (xy.y() > m_boundsMax.y())
        m_boundsMax.y() = xy.y();
}


bool mdn::Mdn2d::internal_checkDigit(const Coord& xy, Digit value) const {
    if (value < m_dbase and value > -m_dbase) {
        return true;
    }
    throw OutOfRange(xy, value, m_dbase);
}


int mdn::Mdn2d::internal_purgeExcessDigits() {
    if (!locked_hasBounds()) {
        // No digits to bother keeping
        return;
    }

    Coord currentSpan = m_boundsMax - m_boundsMin;
    std::unordered_set<Coord> purgeSet;
    int purgeX = currentSpan.x() - m_precision;
    if (purgeX > 0) {
        int minX = m_boundsMax.x() - m_precision;
        for (const auto& [x, coords] : m_xIndex) {
            if (x >= minX) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    int purgeY = currentSpan.y() - m_precision;
    if (purgeY > 0) {
        int minY = m_boundsMax.y() - m_precision;
        for (const auto& [y, coords] : m_yIndex) {
            if (y >= minY) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }

    if (!purgeSet.empty()) {
        std::ostringstream oss;
        oss << "Purging " << purgeSet.size() << " low digit values, below numerical precision ";
        oss << m_precision << std::endl;
        Logger::instance().debug(oss.str());
        locked_setToZero(purgeSet);
    }
    return purgeSet.size();
}


void mdn::Mdn2d::internal_updateBounds() {
    if (m_xIndex.empty() || m_yIndex.empty()) {
        m_boundsMin = Coord({constants::intMax, constants::intMax});
        m_boundsMax = Coord({constants::intMin, constants::intMin});
    } else {
        auto itMinX = m_xIndex.cbegin();
        auto itMaxX = m_xIndex.crbegin();
        auto itMinY = m_yIndex.cbegin();
        auto itMaxY = m_yIndex.crbegin();
        m_boundsMin = {itMinX->first, itMinY->first};
        m_boundsMax = {itMaxX->first, itMaxY->first};
    }
}


void mdn::Mdn2d::internal_checkFraxis(Fraxis& fraxis) const {
    if (fraxis == Fraxis::Default) {
        fraxis = m_defaultFraxis;
    }
}


void mdn::Mdn2d::internal_fraxis(const Coord& xy, double f, int dX, int dY, int c) {
    #ifdef MDN_DEBUG
        if (fraction < -1.0 || fraction > 1.0)
        {
            throw std::invalid_argument(
                "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
                "."
            );
        }
    #endif

    // Debug
    int count = 0;

    // Calculate next digit
    Coord xyWorking(xy);
    while (
        (locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below) &&
        (abs(f) > m_epsilon)
        #ifdef MDN_DEBUG
            && ++count < 100
        #endif
    ) {
        f *= static_cast<double>(m_base);
        Digit d(f);
        if (d != 0) {
            locked_add(xyWorking, d);
            internal_fraxisCascade(xyWorking, d, c);
        }
        f -= d;
        xyWorking.translate(dX, dY);
    }
}


void mdn::Mdn2d::internal_fraxisCascade(const Coord& xy, Digit d, int c)
{
    Coord xyNext = xy.copyTranslate(c, -c);
    d *= -1;
    if (locked_checkPrecisionWindow(xyNext) == PrecisionStatus::Below) {
        // Cascade done
        return;
    }
    locked_add(xyNext, d);
    internal_fraxisCascade(xyNext, d, c);
}


// mdn::Mdn2d& mdn::Mdn2d::internal_plusEquals(const Mdn2d& rhs, int scalar) {
//     for (const auto& [xy, digit] : rhs.m_raw) {
//         int id = static_cast<int>(digit);
//         locked_add(xy, id*scalar);
//     }
// }


mdn::Mdn2d& mdn::Mdn2d::internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const {
    Mdn2d temp(*this);
    auto tempLock = temp.lockWriteable();
    temp.locked_multiply(value);
    temp.shift(shiftXY);
    return temp;
}


void mdn::Mdn2d::internal_polymorphicScanAndFix() {
    std::vector<Coord> required;
    int maxCarryoverScans = 10;
    bool failed = true;
    for (int i = 0; i < maxCarryoverScans; ++i) {
        required.clear();
        m_polymorphicNodes.clear();
        for (const auto& [xy, digit] : m_raw) {
            switch(locked_checkCarryover(xy)) {
                case Carryover::Required:
                    required.push_back(xy);
                    break;
                case Carryover::Optional:
                    m_polymorphicNodes.insert(xy);
                    break;
                default:
                    break;
            }
        }
        if (required.size()) {
            for (const Coord& xy : required) {
                internal_oneCarryover(xy);
            }
        } else {
            failed = false;
            break;
        }
    }
    if (failed) {
        std::ostringstream oss;
        oss << "Internal error: carryover scans did not stabilise after " << maxCarryoverScans;
        oss << " attempts." << std::endl;
        Logger::instance().warn(oss.str());
    }
    m_polymorphicNodes_event = m_event;
}


void mdn::Mdn2d::internal_polymorphicScan() const {
    int nRequired = 0;
    m_polymorphicNodes.clear();
    for (const auto& [xy, digit] : m_raw) {
        switch(locked_checkCarryover(xy)) {
            case Carryover::Required:
                ++nRequired;
                break;
            case Carryover::Optional:
                m_polymorphicNodes.insert(xy);
                break;
            default:
                break;
        }
    }
    if (nRequired) {
        std::ostringstream oss;
        oss << "Internal error: found " << nRequired << " required carryovers during scan." << endl;
        oss << "MDN is in an invalid state." << std::endl;
        Logger::instance().warn(oss.str());
    }
    m_polymorphicNodes_event = m_event;
}


void mdn::Mdn2d::internal_oneCarryover(const Coord& xy) {
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_base;
    if (ip > 0) {
        ip -= m_base;
        iy += 1;
        ix += 1;
    } else {
        ip += m_base;
        iy -= 1;
        ix -= 1;
    }
    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
}


void mdn::Mdn2d::internal_ncarryover(const Coord& xy) {
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    #ifdef MDN_DEBUG
        if (abs(p) < m_dbase) {
            std::ostringstream oss;
            oss << "Invalid carryover requested at " << xy << ": value (" << p << ") must exceed "
                << "base (" << m_dbase << ")";
            throw IllegalOperation(oss.str());
        }
    #endif
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_base;
    ip -= nCarry * m_base;
    iy += nCarry;
    ix += nCarry;

    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
}
