#include "Mdn2d.h"

#include <cmath>
#include <stdexcept>
#include <sstream>
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Mdn2d::Mdn2d(int base)
:
    m_base(base)
{}


mdn::Mdn2d::Mdn2d(int base, int initVal)
:
    m_base(base)
{
    locked_addInteger(Coord({0, 0}), initVal);
}


mdn::Mdn2d::Mdn2d(int base, double initVal, Fraxis fraxis)
:
    m_base(base)
{
    locked_addReal(COORD_ORIGIN, initVal, fraxis);
}


void mdn::Mdn2d::addReal(int x, int y, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_addReal(Coord({x, y}), realNum, fraxis);
}


void mdn::Mdn2d::addReal(const Coord& xy, double realNum, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_addReal(xy, realNum, fraxis);
}


void mdn::Mdn2d::addInteger(int x, int y, int value) {
    auto lock = lockWriteable();
    locked_addInteger(Coord({x, y}), value);
}


void mdn::Mdn2d::addInteger(const Coord& xy, int value) {
    auto lock = lockWriteable();
    locked_addInteger(xy, value);
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    auto lock = lockWriteable();
    locked_addFraxis(xy, fraction, fraxis);
}


mdn::Digit mdn::Mdn2d::getValue(int x, int y) const {
    auto lock = lockReadOnly();
    return locked_getValue(Coord({x, y}));
}


mdn::Digit mdn::Mdn2d::getValue(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_getValue(xy);
}


std::vector<mdn::Digit> mdn::Mdn2d::getRow(int y) const {
    auto lock = lockReadOnly();
    return locked_getRow(y);
}


std::vector<mdn::Digit> mdn::Mdn2d::getCol(int x) const {
    auto lock = lockReadOnly();
    return locked_getCol(x);
}


void mdn::Mdn2d::fillRow(int y, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_fillRow(y, digits);
}


void mdn::Mdn2d::fillCol(int x, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_fillCol(x, digits);
}


void mdn::Mdn2d::setValue(int x, int y, int value) {
    auto lock = lockWriteable();
    locked_setValue(Coord({x, y}), value);
}


void mdn::Mdn2d::setValue(const Coord& xy, int value) {
    auto lock = lockWriteable();
    locked_setValue(xy, value);
}


void mdn::Mdn2d::clear() {
    auto lock = lockWriteable();
    locked_clear();
}


std::string mdn::Mdn2d::toString() const {
    auto lock = lockReadOnly();
    return locked_toString();
}


std::vector<std::string> mdn::Mdn2d::toStringRows(bool reverse) const {
    auto lock = lockReadOnly();
    return locked_toStringRows(reverse);
}


std::vector<std::string> mdn::Mdn2d::toStringCols(bool reverse) const {
    auto lock = lockReadOnly();
    return locked_toStringCols(reverse);
}


void mdn::Mdn2d::carryOver(int x, int y)
{
    carryOver(Coord({x, y}));
}


void mdn::Mdn2d::carryOver(const Coord& xy)
{
    // auto& val = operator()
}


void mdn::Mdn2d::rebuildMetadata() const {
    std::unique_lock lock(m_mutex);
    locked_clearMetadata();

    for (const auto& [coord, digit] : m_raw) {
        if (digit != 0) {
            const int x = coord.first;
            const int y = coord.second;
            ++m_xCounts[x];
            ++m_yCounts[y];
        }
    }

    m_boundsMin = Coord({0, 0});
    m_boundsMax = Coord({0, 0});
    if (m_xCounts.size() == 0)
    {
        // No non-zeros
        return;
    }

    auto itMinX = m_xCounts.cbegin();
    auto itMaxX = m_xCounts.crbegin();
    auto itMinY = m_yCounts.cbegin();
    auto itMaxY = m_yCounts.crbegin();
    m_boundsMin = {itMinX->first, itMinY->first};
    m_boundsMax = {itMinX->second, itMinY->second};
}


mdn::Digit mdn::Mdn2d::operator()(int x, int y) const {
    return operator()(Coord(x, y));
}


mdn::Digit mdn::Mdn2d::operator()(const Coord& xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        return Digit(0);
    }
    return it->second;
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    for (const auto& [coord, val] : rhs.m_raw) {
        m_raw[coord] += val;
    }
    return *this;
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


mdn::Mdn2d::WritableLock mdn::Mdn2d::lockWriteable() const {
    return std::unique_lock(m_mutex);
}


mdn::Mdn2d::ReadOnlyLock mdn::Mdn2d::lockReadOnly() const {
    return std::shared_lock(m_mutex);
}


void mdn::Mdn2d::purgeZeroes() {
    for (auto it = m_raw.begin(); it != m_raw.end(); ) {
        if (it->second == 0) {
            it = m_raw.erase(it);
        } else {
            ++it;
        }
    }
}


mdn::Digit& mdn::Mdn2d::operator()(int x, int y) {
    return operator()(Coord(x, y));
}


mdn::Digit& mdn::Mdn2d::operator()(const Coord& xy) {
    return m_raw[xy];
}


void mdn::Mdn2d::locked_clear() {
}


void mdn::Mdn2d::locked_clearMetadata() const {
    m_xCounts.clear();
    m_yCounts.clear();
    m_boundsMin = {0, 0}; // Find a better 'null' value for bounds
    m_boundsMax = {0, 0}; // Find a better 'null' value for bounds
}


bool mdn::Mdn2d::locked_checkBounds(const Coord& xy) {
    Coord maxLimit = m_boundsMin + m_maxSpan;
    Coord minLimit = m_boundsMax + m_maxSpan;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    if (purgeX > 0) {
        // Lower magnitude entries need to be purged

    }


spn = 8
-2  ...  5
mnb
lmx 6
xy.x = 6 all ok
x = 7, purge 1
x = 8 purge 2
purge = x - lmx

    int upperX = m_boundsMin.first + m_maxSpan;
    int lowerX = m_boundsMax.first - m_maxSpan;

    m_boundsMin
m_boundsMax
}


void mdn::Mdn2d::locked_updateBounds() {
    auto itMinX = m_xIndex.cbegin();
    auto itMaxX = m_xIndex.crbegin();
    auto itMinY = m_yIndex.cbegin();
    auto itMaxY = m_yIndex.crbegin();
    m_boundsMin = {itMinX->first, itMinY->first};
    m_boundsMax = {itMaxX->first, itMaxY->first};
}


std::string mdn::Mdn2d::locked_toString() const {
    std::vector<std::string> rows = locked_toStringRows(true);
    return Tools::joinArray(rows, "\n");
}


std::vector<std::string> mdn::Mdn2d::locked_toStringRows(bool reverse=true) const {
    int yStart, yEnd;
    if (reverse) {
        yStart = m_boundsMax.y();
        yEnd = m_boundsMin.y();
    } else {
        yStart = m_boundsMin.y();
        yEnd = m_boundsMax.y();
    }
    std::vector<Digit> digits;
    for (int y = yStart; y < yEnd; ++y) {
        locked_fillRow(y, digits);
        ////// WORKING HERE
    }




    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x() + 1; // one past the end, prevent fencepost
    int xCount = xEnd - xStart;
    int yStart, yEnd;
    if (reverse) {
        yStart = m_boundsMax.y();
        yEnd = m_boundsMin.y();
    } else {
        yStart = m_boundsMin.y();
        yEnd = m_boundsMax.y();
    }

    std::vector<Digit> digits(xCount);
    for (int y = yStart; y < yEnd; ++y) {
        // Init digits for this row
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

// mutable Coord m_boundsMin;
// mutable Coord m_boundsMax;
// std::map<int, std::unordered_set<Coord>> m_xIndex
// std::map<int, std::unordered_set<Coord>> m_yIndex
}



std::vector<mdn::Digit> mdn::Mdn2d::locked_getRow(int y) const {
    std::vector<Digit> digits;
    locked_fillRow(y, digits);
}


std::vector<mdn::Digit> mdn::Mdn2d::locked_getCol(int x) const {
    std::vector<Digit> digits;
    locked_fillCol(x, digits);
}


void mdn::Mdn2d::locked_fillRow(int y, std::vector<Digit>& digits) const {
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


void mdn::Mdn2d::locked_fillCol(int x, std::vector<Digit>& digits) const {
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




std::vector<std::string> mdn::Mdn2d::locked_toStringCols(bool reverse=true) const {
}


mdn::Digit mdn::Mdn2d::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    return it != m_raw.end() ? it->second : 0;
}


void mdn::Mdn2d::locked_setValue(const Coord& xy, int value) {
    #ifdef MDN_DEBUG
        if (value >= m_base || value <= -m_base) {
            mdn::OutOfRange ex(xy, value, m_base);
            Logger::instance().error(ex.what());
            throw ex;
        }
    #endif

    // Sparse storage does not store zeroes
    if (value == 0)
    {
        setToZero(xy);
        return;
    }

    // Non-zero entry - set / overwrite
    m_raw[xy] = value;
}


void mdn::Mdn2d::locked_addReal(const Coord& xy, double realNum, Fraxis fraxis) {
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    locked_addInteger(xy, intPart);
    locked_addFraxis(xy, fracPart, fraxis);
}


void mdn::Mdn2d::locked_addInteger(const Coord& xy, int value) {
    int div;
    if (value == 0) {
        return;
    }

    // Grad current value, cast to int
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // xy has zero value
        int sum = value;
        div = sum / m_base;
        int rem = sum % m_base;
        if (rem != 0) {
            // Check bounds
            locked_checkBounds(xy);
            m_raw[xy] = rem;
        }
    } else {
        // xy is non-zero
        int curVal = it->second;
        int sum = value + curVal;
        div = sum / m_base;
        int rem = sum % m_base;
        if (rem != 0) {
            m_raw[xy] = rem;
        }
    }
    if (div != 0) {
        locked_addInteger(Coord({xy.first+1, xy.second}), div);
        locked_addInteger(Coord({xy.first, xy.second+1}), div);
    }
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


mdn::Digit* mdn::Mdn2d::getPtr(const Coord& xy)
{
    auto iter = m_raw.find(xy);
    if (iter == m_raw.end()) {
        return nullptr;
    }
    return &(iter->second);
}


void mdn::Mdn2d::setToZero(const Coord& xy) {
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
        locked_updateBounds();
    }
}
