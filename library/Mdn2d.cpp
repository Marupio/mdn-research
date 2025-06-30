#include "Mdn2d.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <sstream>
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


const int mdn::Mdn2d::m_intMin = std::numeric_limits<int>::min();
const int mdn::Mdn2d::m_intMax = std::numeric_limits<int>::max();


mdn::Mdn2d::Mdn2d(int base, int maxSpan)
:
    m_base(base),
    m_maxSpan(maxSpan),
    m_boundsMin({m_intMax, m_intMax}),
    m_boundsMax({m_intMin, m_intMin})
{}


mdn::Mdn2d::Mdn2d(int base, int maxSpan, int initVal)
:
    m_base(base),
    m_maxSpan(maxSpan),
    m_boundsMin({m_intMax, m_intMax}),
    m_boundsMax({m_intMin, m_intMin})
{
    locked_addInteger(Coord({0, 0}), initVal);
}


mdn::Mdn2d::Mdn2d(int base, int maxSpan, double initVal, Fraxis fraxis)
:
    m_base(base),
    m_maxSpan(maxSpan),
    m_boundsMin({m_intMax, m_intMax}),
    m_boundsMax({m_intMin, m_intMin})
{
    locked_addReal(COORD_ORIGIN, initVal, fraxis);
}


mdn::Mdn2d::Mdn2d(const Mdn2d& other):
    m_base(other.m_base),
    m_maxSpan(other.m_maxSpan)
{
    auto lock = other.lockReadOnly();

    m_raw = other.m_raw;
    m_xIndex = other.m_xIndex;
    m_yIndex = other.m_yIndex;
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();

        if (m_base != other.m_base) {
            throw BaseMismatch(m_base, other.m_base);
        }
        m_maxSpan = other.m_maxSpan;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
    }
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other) noexcept
    : m_base(other.m_base),
      m_maxSpan(other.m_maxSpan)
{
    auto lock = other.lockWriteable();

    m_raw = std::move(other.m_raw);
    m_xIndex = std::move(other.m_xIndex);
    m_yIndex = std::move(other.m_yIndex);
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockWriteable();

        if (m_base != other.m_base) {
            throw BaseMismatch(m_base, other.m_base);
        }
        m_maxSpan = other.m_maxSpan;
        m_raw = std::move(other.m_raw);
        m_xIndex = std::move(other.m_xIndex);
        m_yIndex = std::move(other.m_yIndex);
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
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
        return Digit(0);
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
}


void mdn::Mdn2d::locked_clear() {
    m_raw.clear();
    internal_clearMetadata();
}


void mdn::Mdn2d::setToZero(const Coord& xy) {
    auto lock = lockWriteable();
    locked_setToZero(xy);
}


void mdn::Mdn2d::locked_setToZero(const Coord& xy) {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // Already zero
        return;
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
    }

    if (coordsAlongY.size() == 0) {
        m_yIndex.erase(yit);
    }
    if (checkBounds) {
        // Bounds may have changed
        internal_updateBounds();
    }
}


void mdn::Mdn2d::setToZero(const std::unordered_set<Coord>& coords) {
    auto lock = lockWriteable();
    locked_setToZero(coords);
}


void mdn::Mdn2d::locked_setToZero(const std::unordered_set<Coord>& purgeSet) {
    // Step 1: Erase from m_raw
    for (const Coord& coord : purgeSet) {
        m_raw.erase(coord);
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
}


void mdn::Mdn2d::setValue(const Coord& xy, Digit value) {
    auto lock = lockWriteable();
    locked_setValue(xy, value);
}


void mdn::Mdn2d::setValue(const Coord& xy, int value) {
    auto lock = lockWriteable();
    locked_setValue(xy, Digit(value));
}


void mdn::Mdn2d::setValue(const Coord& xy, long value) {
    auto lock = lockWriteable();
    locked_setValue(xy, Digit(value));
}


void mdn::Mdn2d::setValue(const Coord& xy, long long value) {
    auto lock = lockWriteable();
    locked_setValue(xy, Digit(value));
}


void mdn::Mdn2d::locked_setValue(const Coord& xy, Digit value) {
    // TODO
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockReadOnly();
    locked_plus(rhs, ans);
}


void mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
    // TODO
}


void mdn::Mdn2d::minus(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "minus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockReadOnly();
    locked_minus(rhs, ans);
}


void mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
}


void mdn::Mdn2d::multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "multiply operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockReadOnly();
    locked_multiply(rhs, ans);
}


void mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    // TODO
}


void mdn::Mdn2d::divide(const Mdn2d& rhs, Mdn2d& ans) const {
    assertNotSelf(ans, "divide operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockReadOnly();
    locked_divide(rhs, ans);
}


void mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans) const {
    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_add(xy, double(realNum), fraxis);
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_add(xy, realNum, fraxis);
}


void mdn::Mdn2d::locked_add(const Coord& xy, double realNum, Fraxis fraxis) {
    // TODO
}


void mdn::Mdn2d::subtract(const Coord& xy, float realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_subtract(xy, double(realNum), fraxis);
}


void mdn::Mdn2d::subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_subtract(xy, realNum, fraxis);
}


void mdn::Mdn2d::locked_subtract(const Coord& xy, double realNum, Fraxis fraxis) {
    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_add(xy, value)
}


void mdn::Mdn2d::locked_add(const Coord& xy, Digit value) {
    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, int value) {
    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, long value) {
    // TODO
}


void mdn::Mdn2d::add(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_add(xy, value);
}


void mdn::Mdn2d::locked_add(const Coord& xy, long long value) {
    // TODO
}


void mdn::Mdn2d::subtract(const Coord& xy, Digit value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_subtract(xy, value)
}


void mdn::Mdn2d::locked_subtract(const Coord& xy, Digit value) {
    // TODO
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_subtract(xy, value);
}


void mdn::Mdn2d::locked_subtract(const Coord& xy, int value) {
    // TODO
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_subtract(xy, value);
}


void mdn::Mdn2d::locked_subtract(const Coord& xy, long value) {
    // TODO
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, Fraxis unused) {
    auto lock = lockWriteable();
    locked_subtract(xy, value);
}


void mdn::Mdn2d::locked_subtract(const Coord& xy, long long value) {
    // TODO
}


void mdn::Mdn2d::addFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_addFraxis(xy, double(fraction), fraxis);
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_addFraxis(xy, fraction, fraxis);
}


void mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    #ifdef MDN_DEBUG
        if (fraction < -1.0 || fraction > 1.0)
        {
            std::ostringstream oss;
            oss << "fraction out of range (-1.0 .. 1.0), got " << fraction;
            Logger::instance().error(oss.str());
        }
    #endif

    switch(fraxis)
    {
        case Fraxis::X:
            locked_addFraxisX(xy, fraction);
            break;
        case Fraxis::Y:
            locked_addFraxisY(xy, fraction);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            Logger::instance().error(oss.str());
            break;
    }
}


void mdn::Mdn2d::locked_addFraxisX(const Coord& xy, double fraction) {
    // TODO
}


void mdn::Mdn2d::locked_addFraxisY(const Coord& xy, double fraction) {
    // TODO
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_subtractFraxis(xy, double(fraction), fraxis);
}


void mdn::Mdn2d::subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_subtractFraxis(xy, fraction, fraxis);
}


void mdn::Mdn2d::locked_subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    // TODO
}


void mdn::Mdn2d::locked_subtractFraxisX(const Coord& xy, double fraction) {
    // TODO
}


void mdn::Mdn2d::locked_subtractFraxisY(const Coord& xy, double fraction) {
    // TODO
}


void mdn::Mdn2d::multiply(Digit value) {
    auto lock = lockWriteable();
    locked_multiply(value);
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    locked_multiply(Digit(value));
}


void mdn::Mdn2d::multiply(long value) {
    auto lock = lockWriteable();
    locked_multiply(Digit(value));
}


void mdn::Mdn2d::multiply(long long value) {
    auto lock = lockWriteable();
    locked_multiply(Digit(value));
}


void mdn::Mdn2d::locked_multiply(Digit value) {
    // TODO
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

    std::vector<std::string> out;
    out.reserve(yCount+1);
    std::vector<Digit> digits;
    for (int y = yStart; y < yEnd; ++y) {
        // First, are we at the yDigit line?
        if (y == 0) {
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
    // TODO
}


void mdn::Mdn2d::carryOver(const Coord& xy)
{
    auto lock = lockWriteable();
    locked_carryOver(xy);
}


void mdn::Mdn2d::locked_carryOver(const Coord& xy) {
    // TODO
}


void mdn::Mdn2d::shiftRight(int nDigits) {
    auto lock = lockWriteable();
    locked_shiftRight(nDigits);
}


void mdn::Mdn2d::locked_shiftRight(int nDigits) {
    // TODO
}


void mdn::Mdn2d::shiftLeft(int nDigits) {
    auto lock = lockWriteable();
    locked_shiftLeft(nDigits);
}


void mdn::Mdn2d::locked_shiftLeft(int nDigits) {
    // TODO
}


void mdn::Mdn2d::shiftUp(int nDigits) {
    auto lock = lockWriteable();
    locked_shiftUp(nDigits);
}


void mdn::Mdn2d::locked_shiftUp(int nDigits) {
    // TODO
}


void mdn::Mdn2d::shiftDown(int nDigits) {
    auto lock = lockWriteable();
    locked_shiftDown(nDigits);
}


void mdn::Mdn2d::locked_shiftDown(int nDigits) {
    // TODO
}


void mdn::Mdn2d::transpose() {
    auto lock = lockWriteable();
    locked_transpose();
}


void mdn::Mdn2d::locked_transpose() {
    // TODO
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
        m_boundsMin.x() == m_intMax ||
        m_boundsMin.y() == m_intMax ||
        m_boundsMax.x() == m_intMin ||
        m_boundsMax.y() == m_intMin
    );

    return !invalid;
}


int mdn::Mdn2d::getPrecision() const {
    auto lock = lockReadOnly();
    return locked_getPrecision();
}


int mdn::Mdn2d::locked_getPrecision() const {
    return m_maxSpan;
}


void mdn::Mdn2d::setPrecision(int newMaxSpan) {
    auto lock = lockWriteable();
    locked_setPrecision(newMaxSpan);
}


void mdn::Mdn2d::locked_setPrecision(int newMaxSpan) {
    if (newMaxSpan < 1) {
        throw std::invalid_argument("maxSpan, Mdn2d numerical precision, cannot be less than 1");
    }
    else if (newMaxSpan >= m_maxSpan) {
        m_maxSpan = newMaxSpan;
        return;
    }

    // New precision is *less* than the old one - may not be able to keep all our digits
    int oldMaxSpan = m_maxSpan;
    m_maxSpan = newMaxSpan;

    if (!locked_hasBounds()) {
        // No digits to bother keeping
        return;
    }

    Coord currentSpan = m_boundsMax - m_boundsMin;
    std::unordered_set<Coord> purgeSet;
    int purgeX = currentSpan.x() - m_maxSpan;
    if (purgeX > 0) {
        int minX = m_boundsMax.x() - m_maxSpan;
        for (const auto& [x, coords] : m_xIndex) {
            if (x >= minX) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    int purgeY = currentSpan.y() - m_maxSpan;
    if (purgeY > 0) {
        int minY = m_boundsMax.y() - m_maxSpan;
        for (const auto& [y, coords] : m_yIndex) {
            if (y >= minY) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    if (!purgeSet.empty()) {
        std::ostringstream oss;
        oss << "Purging " << purgeSet.size() << " low digit values, below numerical precision ";
        oss << m_maxSpan << std::endl;
        Logger::instance().debug(oss.str());
        locked_setToZero(purgeSet);
    }
}


// void mdn::Mdn2d::purgeZeroes() {
//     for (auto it = m_raw.begin(); it != m_raw.end(); ) {
//         if (it->second == 0) {
//             it = m_raw.erase(it);
//         } else {
//             ++it;
//         }
//     }
// }




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
    Mdn2d temp(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    locked_plus(rhs, temp);
    operator=(temp);
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    for (const auto& [coord, val] : rhs.m_raw) {
        m_raw[coord] -= val;
    }
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    // Stub: implement proper MDN multiplication logic
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    // Stub: implement proper MDN division logic
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    for (auto& [_, val] : m_raw) {
        val *= scalar;
    }
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(int scalar) {
    for (auto& [_, val] : m_raw) {
        val /= scalar;
    }
    return *this;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    // TODO - Take into account polymorphic states
    return m_raw == rhs.m_raw;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    // TODO - Take into account polymorphic states
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
    m_boundsMin = Coord({m_intMax, m_intMax});
    m_boundsMax = Coord({m_intMin, m_intMin});

    m_xIndex.clear();
    m_yIndex.clear();
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


bool mdn::Mdn2d::internal_checkBounds(const Coord& xy) {
    // minLimit - below this and the new value should not be added
    Coord minLimit = m_boundsMax - m_maxSpan;

    // Check under limit
    if (xy.x() < minLimit.x() || xy.y() < minLimit.y()) {
        // Value is under-limit, do not add
        return false;
    }

    // Check over limit
    std::unordered_set<Coord> purgeSet;
    Coord newMinLimit = m_boundsMin;
    // maxLimit - above this and we need to purge existing values
    Coord maxLimit = m_boundsMin + m_maxSpan;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    if (purgeX > 0) {
        newMinLimit.x() += purgeX;
        int minX = newMinLimit.x();
        for (const auto& [x, coords] : m_xIndex) {
            if (x >= minX) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    int purgeY = xy.y() - maxLimit.y();
    if (purgeY > 0) {
        newMinLimit.y() += purgeY;
        int minY = newMinLimit.y();
        for (const auto& [y, coords] : m_yIndex) {
            if (y >= minY) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }

    if (!purgeSet.empty()) {
        std::ostringstream oss;
        oss << "Purging " << purgeSet.size() << " low digit values, below numerical precision ";
        oss << m_maxSpan << std::endl;
        Logger::instance().debug(oss.str());
        locked_setToZero(purgeSet);
    }
    return true;
}


void mdn::Mdn2d::internal_updateBounds() {
    if (m_xIndex.empty() || m_yIndex.empty()) {
        m_boundsMin = Coord({m_intMax, m_intMax});
        m_boundsMax = Coord({m_intMin, m_intMin});
    } else {
        auto itMinX = m_xIndex.cbegin();
        auto itMaxX = m_xIndex.crbegin();
        auto itMinY = m_yIndex.cbegin();
        auto itMaxY = m_yIndex.crbegin();
        m_boundsMin = {itMinX->first, itMinY->first};
        m_boundsMax = {itMaxX->first, itMaxY->first};
    }
}


mdn::Digit* mdn::Mdn2d::getPtr(const Coord& xy)
{
    auto iter = m_raw.find(xy);
    if (iter == m_raw.end()) {
        return nullptr;
    }
    return &(iter->second);
}
