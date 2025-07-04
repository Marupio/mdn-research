#include "Mdn2d.h"
#include "Mdn2dBase.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Mdn2d mdn::Mdn2dBase::NewInstance(Mdn2dConfig config) {
    return Mdn2d(config);
}


mdn::Mdn2d mdn::Mdn2dBase::Duplicate(const Mdn2d& other) {
    return Mdn2d(other);
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dConfig config)
:
    m_config(config),
    m_boundsMin({constants::intMax, constants::intMax}),
    m_boundsMax({constants::intMin, constants::intMin}),
    m_event(0)
{}


mdn::Mdn2dBase::Mdn2dBase(const Mdn2dBase& other):
    m_config(other.m_config),
    m_event(0)
{
    auto lock = other.lockReadOnly();
    m_raw = other.m_raw;
    m_xIndex = other.m_xIndex;
    m_yIndex = other.m_yIndex;
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(const Mdn2dBase& other) {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();

        if (m_config.base() != other.m_config.base()) {
            throw BaseMismatch(m_config.base(), other.m_config.base());
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = other.m_raw;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        modified();
    }
    return *this;
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dBase&& other) noexcept :
    m_config(other.m_config),
    m_event(0)
{
    auto lock = other.lockWriteable();
    m_raw = std::move(other.m_raw);
    m_xIndex = std::move(other.m_xIndex);
    m_yIndex = std::move(other.m_yIndex);
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(Mdn2dBase&& other) noexcept {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();

        if (m_config.base() != other.m_config.base()) {
            throw BaseMismatch(m_config.base(), other.m_config.base());
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = std::move(other.m_raw);
        m_xIndex = std::move(other.m_xIndex);
        m_yIndex = std::move(other.m_yIndex);
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        modified();
    }
    return *this;
}


mdn::Digit mdn::Mdn2dBase::getValue(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_getValue(xy);
}


mdn::Digit mdn::Mdn2dBase::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        return static_cast<Digit>(0);
    }
    return it->second;
}


std::vector<mdn::Digit> mdn::Mdn2dBase::getRow(int y) const {
    auto lock = lockReadOnly();
    return locked_getRow(y);
}


std::vector<mdn::Digit> mdn::Mdn2dBase::locked_getRow(int y) const {
    std::vector<Digit> digits;
    locked_getRow(y, digits);
}


void mdn::Mdn2dBase::getRow(int y, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_getRow(y, digits);
}


void mdn::Mdn2dBase::locked_getRow(int y, std::vector<Digit>& digits) const {
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


std::vector<mdn::Digit> mdn::Mdn2dBase::getCol(int x) const {
    auto lock = lockReadOnly();
    return locked_getCol(x);
}


std::vector<mdn::Digit> mdn::Mdn2dBase::locked_getCol(int x) const {
    std::vector<Digit> digits;
    locked_getCol(x, digits);
}


void mdn::Mdn2dBase::getCol(int x, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    locked_getCol(x, digits);
}


void mdn::Mdn2dBase::locked_getCol(int x, std::vector<Digit>& digits) const {
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


void mdn::Mdn2dBase::clear() {
    auto lock = lockWriteable();
    locked_clear();
    modified();
}


void mdn::Mdn2dBase::locked_clear() {
    m_raw.clear();
    internal_clearMetadata();
}


bool mdn::Mdn2dBase::setToZero(const Coord& xy) {
    auto lock = lockWriteable();
    modified();
    return locked_setToZero(xy);
}


bool mdn::Mdn2dBase::locked_setToZero(const Coord& xy) {
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


int mdn::Mdn2dBase::setToZero(const std::unordered_set<Coord>& coords) {
    auto lock = lockWriteable();
    modified();
    return locked_setToZero(coords);
}


int mdn::Mdn2dBase::locked_setToZero(const std::unordered_set<Coord>& purgeSet) {
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


bool mdn::Mdn2dBase::setValue(const Coord& xy, Digit value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, value);
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, int value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long long value) {
    auto lock = lockWriteable();
    modified();
    return locked_setValue(xy, static_cast<Digit>(value));
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, Digit value) {
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


std::string mdn::Mdn2dBase::toString() const {
    auto lock = lockReadOnly();
    return locked_toString();
}


std::string mdn::Mdn2dBase::locked_toString() const {
    std::vector<std::string> rows = locked_toStringRows();
    return Tools::joinArray(rows, "\n", true);
}


std::vector<std::string> mdn::Mdn2dBase::toStringRows() const {
    auto lock = lockReadOnly();
    return locked_toStringRows();
}


std::vector<std::string> mdn::Mdn2dBase::locked_toStringRows() const {
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


std::vector<std::string> mdn::Mdn2dBase::toStringCols() const {
    auto lock = lockReadOnly();
    return locked_toStringCols();
}


std::vector<std::string> mdn::Mdn2dBase::locked_toStringCols() const {
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
    for (int x = xStart; x < xEnd; ++x) {
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


void mdn::Mdn2dBase::rebuildMetadata() const {
    auto lock = lockWriteable();
    locked_rebuildMetadata();
}


void mdn::Mdn2dBase::locked_rebuildMetadata() const {
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


bool mdn::Mdn2dBase::hasBounds() const {
    auto lock = lockReadOnly();
    return locked_hasBounds();
}


bool mdn::Mdn2dBase::locked_hasBounds() const {
    bool invalid = (
        m_boundsMin.x() == constants::intMax ||
        m_boundsMin.y() == constants::intMax ||
        m_boundsMax.x() == constants::intMin ||
        m_boundsMax.y() == constants::intMin
    );

    return !invalid;
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2dBase::getBounds() const {
    auto lock = ReadOnlyLock();
    return locked_getBounds();
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2dBase::locked_getBounds() const {
    return std::pair<Coord, Coord>(m_boundsMin, m_boundsMax);
}


int mdn::Mdn2dBase::getPrecision() const {
    auto lock = lockReadOnly();
    return locked_getPrecision();
}


int mdn::Mdn2dBase::locked_getPrecision() const {
    return m_config.precision();
}


int mdn::Mdn2dBase::setPrecision(int newPrecision) {
    auto lock = lockWriteable();
    modified();
    return locked_setPrecision(newPrecision);
}


int mdn::Mdn2dBase::locked_setPrecision(int newPrecision) {
    int oldPrecision = m_config.precision();
    m_config.setPrecision(newPrecision);

    if (oldPrecision < newPrecision)
    {
        return internal_purgeExcessDigits();
    }
}


mdn::PrecisionStatus mdn::Mdn2dBase::checkPrecisionWindow(const Coord& xy) const {
    auto lock = ReadOnlyLock();
    return locked_checkPrecisionWindow(xy);
}


mdn::PrecisionStatus mdn::Mdn2dBase::locked_checkPrecisionWindow(const Coord& xy) const {
    if (!locked_hasBounds()) {
        return PrecisionStatus::Inside;
    }
    int precision = m_config.precision();

    // minLimit - below this and the new value should not be added
    Coord minLimit = m_boundsMax - precision;

    // Check under limit
    if (xy.x() < minLimit.x() || xy.y() < minLimit.y()) {
        // Value is under-limit, do not add
        return PrecisionStatus::Below;
    }

    // Check over limit
    std::unordered_set<Coord> purgeSet;
    // maxLimit - above this and we need to purge existing values
    Coord maxLimit = m_boundsMin + precision;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    int purgeY = xy.y() - maxLimit.y();
    if (purgeX > 0 || purgeY > 0) {
        return PrecisionStatus::Above;
    }
    return PrecisionStatus::Inside;
}


void mdn::Mdn2dBase::modified(){
    m_event++;
}


mdn::Mdn2dBase::WritableLock mdn::Mdn2dBase::lockWriteable() const {
    return std::unique_lock(m_mutex);
}


mdn::Mdn2dBase::ReadOnlyLock mdn::Mdn2dBase::lockReadOnly() const {
    return std::shared_lock(m_mutex);
}


void mdn::Mdn2dBase::assertNotSelf(Mdn2dBase& that, const std::string& description) const {
    if (this == &that) {
        throw IllegalSelfReference(description);
    }
}


void mdn::Mdn2dBase::internal_clearMetadata() const {
    m_boundsMin = Coord({constants::intMax, constants::intMax});
    m_boundsMax = Coord({constants::intMin, constants::intMin});

    m_xIndex.clear();
    m_yIndex.clear();
}


void mdn::Mdn2dBase::internal_insertAddress(const Coord& xy) const {
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


bool mdn::Mdn2dBase::internal_checkDigit(const Coord& xy, Digit value) const {
    Digit dbase = m_config.dbase();
    if (value < dbase and value > -dbase) {
        return true;
    }
    throw OutOfRange(xy, value, dbase);
}


int mdn::Mdn2dBase::internal_purgeExcessDigits() {
    if (!locked_hasBounds()) {
        // No digits to bother keeping
        return;
    }

    int precision = m_config.precision();
    Coord currentSpan = m_boundsMax - m_boundsMin;
    std::unordered_set<Coord> purgeSet;
    int purgeX = currentSpan.x() - precision;
    if (purgeX > 0) {
        int minX = m_boundsMax.x() - precision;
        for (const auto& [x, coords] : m_xIndex) {
            if (x >= minX) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    int purgeY = currentSpan.y() - precision;
    if (purgeY > 0) {
        int minY = m_boundsMax.y() - precision;
        for (const auto& [y, coords] : m_yIndex) {
            if (y >= minY) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }

    if (!purgeSet.empty()) {
        std::ostringstream oss;
        oss << "Purging " << purgeSet.size() << " low digit values, below numerical precision ";
        oss << precision << std::endl;
        Logger::instance().debug(oss.str());
        locked_setToZero(purgeSet);
    }
    return purgeSet.size();
}


void mdn::Mdn2dBase::internal_updateBounds() {
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
