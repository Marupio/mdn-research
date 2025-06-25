#include <cmath>
#include <stdexcept>
#include <sstream>
#include "Logger.h"
#include "Mdn2d.h"
#include "MdnException.h"


mdn::Mdn2d::Mdn2d(int base)
:
    m_base(base)
{}


mdn::Mdn2d::Mdn2d(int base, int initVal)
:
    m_base(base)
{
    auto lock = lockWriteable();
    locked_addInteger(Coord({0, 0}), initVal);
}


mdn::Mdn2d::Mdn2d(int base, double initVal, Fraxis fraxis)
:
    m_base(base)
{
    auto lock = lockWriteable();
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
    m_raw[xy] += static_cast<Digit>(value);
}


void mdn::Mdn2d::addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
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
            addFraxisX(COORD_ORIGIN, fraction);
            break;
        case Fraxis::Y:
            addFraxisY(COORD_ORIGIN, fraction);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            Logger::instance().error(oss.str());
            break;
    }
}


void mdn::Mdn2d::addFraxisX(const Coord& xy, double fraction) {
    // TODO add cascading fractional part
}


void mdn::Mdn2d::addFraxisY(const Coord& xy, double fraction) {
    // TODO add cascading fractional part
}


mdn::Digit mdn::Mdn2d::getValue(int x, int y) const {
    auto lock = lockReadOnly();
    return locked_getValue(Coord({x, y}));
}


mdn::Digit mdn::Mdn2d::getValue(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_getValue(xy);
}


void mdn::Mdn2d::setValue(int x, int y, int value) {
    setValue(Coord({x, y}), value);
}


void mdn::Mdn2d::setValue(const Coord& xy, int value) {
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


void mdn::Mdn2d::clear() {
    std::unique_lock lock(m_mutex);
    locked_clear();
}





std::string mdn::Mdn2d::toString() const {
    std::ostringstream oss;
    for (const auto& [coord, val] : m_raw) {
        oss << "(" << coord.first << "," << coord.second << ")=" << static_cast<int>(val) << "\n";
    }
    return oss.str();
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


void mdn::Mdn2d::locked_checkBounds(const Coord& xy) {
    int upperX = m_boundsMin.first + m_maxSpan;
    int lowerX = m_boundsMax.first - m_maxSpan;

    m_boundsMin
m_boundsMax
}


mdn::Digit mdn::Mdn2d::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    return it != m_raw.end() ? it->second : 0;
}


void mdn::Mdn2d::locked_setValue(const Coord& xy, int value) {
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
}


void mdn::Mdn2d::locked_addFraxisX(const Coord& xy, double fraction) {
}


void mdn::Mdn2d::locked_addFraxisY(const Coord& xy, double fraction) {
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
    if (it != m_raw.end())
    {
        // There is currently a non-zero value - erase it
        m_raw.erase(it);

        // Update metadata - reduce counts by one
        auto cxit = m_xCounts.find(xy.first);
        auto cyit = m_yCounts.find(xy.second);
        #ifdef MDN_DEBUG
            // Debug mode - sanity check - row/col counts must be non-zero
            if (cxit == m_xCounts.end() || cyit == m_yCounts.end()) {
                std::ostringstream oss;
                oss << "MDN Metadata invalid: expecting non-zero row/col counts at " << xy << std::endl;
                oss << "rebuilding metadata.";
                Logger::instance().warn(oss.str());
                rebuildMetadata();
            } else
        #endif // MDN_DEBUG
        {
            // Debug - confirmed values are non-zero; Opt - assume they are.
            cxit->second--;
            cyit->second--;
            if (cxit->second == 0) {
                m_xCounts.erase(cxit);
            }
            if (cyit->second == 0) {
                m_yCounts.erase(cyit);
            }
        }
    } // end if (it != m_raw.end())
    return;
}