#include "Mdn2dBase.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <filesystem>

#include "Constants.h"
#include "GlobalConfig.h"
#include "Logger.h"
#include "Mdn2d.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Mdn2d mdn::Mdn2dBase::NewInstance(Mdn2dConfig config) {
    Log_Debug("Creating a NewInstance of Mdn2d");
    return Mdn2d(config);
}


mdn::Mdn2d mdn::Mdn2dBase::Duplicate(const Mdn2d& other) {
    Log_Debug("Duplicating Mdn2d");
    return Mdn2d(other);
}


mdn::Mdn2dBase::Mdn2dBase()
:
    m_config(Mdn2dConfig::static_defaultConfig()),
    m_boundsMin({constants::intMax, constants::intMax}),
    m_boundsMax({constants::intMin, constants::intMin}),
    m_modified(false),
    m_event(0)
{
    Log_Debug3("Constructing null Mdn2dBase");
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dConfig config)
:
    m_config(config),
    m_boundsMin({constants::intMax, constants::intMax}),
    m_boundsMax({constants::intMin, constants::intMin}),
    m_modified(false),
    m_event(0)
{
    Log_Debug3("Constructing Mdn2dBase with config " << config);
}


mdn::Mdn2dBase::Mdn2dBase(const Mdn2dBase& other):
    m_config(other.m_config),
    m_modified(false),
    m_event(0)
{
    Log_Debug3("Constructing Mdn2dBase as copy");
    auto lock = other.lockReadOnly();
    m_raw = other.m_raw;
    m_xIndex = other.m_xIndex;
    m_yIndex = other.m_yIndex;
    m_index = other.m_index;
    m_index = other.m_index;
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(const Mdn2dBase& other) {
    if (this != &other) {
        Log_Debug3("Setting Mdn2dBase equal to other");
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();

        if (m_config.base() != other.m_config.base()) {
            throw BaseMismatch(m_config.base(), other.m_config.base());
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = other.m_raw;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_index = other.m_index;
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        internal_modified();
    } else {
        Log_Warn("Attempting to set Mdn2d equal to itself");
    }
    return *this;
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dBase&& other) noexcept :
    m_config(other.m_config),
    m_modified(false),
    m_event(0)
{
    auto lock = other.lockWriteable();
    Log_Debug3("Move-copying Mdn2dBase");
    m_raw = std::move(other.m_raw);
    m_xIndex = std::move(other.m_xIndex);
    m_yIndex = std::move(other.m_yIndex);
    m_index = std::move(other.m_index);
    m_boundsMin = other.m_boundsMin;
    m_boundsMax = other.m_boundsMax;
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(Mdn2dBase&& other) noexcept {
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();
        Log_Debug3("Setting Mdn2dBase equal to other, move-copy");

        if (m_config.base() != other.m_config.base()) {
            m_config.setInvalid(
                "Base mismatch during move ctor (" + std::to_string(m_config.base()) +
            " != " + std::to_string(other.m_config.base()) + ")"
            );
            std::ostringstream oss;
            oss << "Error: Base mismatch, cannot set these equal, (base " << m_config.base();
            oss << " != " << other.m_config.base() << ")" << std::endl;
            Logger::instance().error(oss.str());
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = std::move(other.m_raw);
        m_xIndex = std::move(other.m_xIndex);
        m_yIndex = std::move(other.m_yIndex);
        m_index = std::move(other.m_index);
        m_boundsMin = other.m_boundsMin;
        m_boundsMax = other.m_boundsMax;
        internal_modified();
    } else {
        Log_Warn("Attempting to set Mdn2d equal to itself, move copying");
    }
    return *this;
}


mdn::Digit mdn::Mdn2dBase::getValue(const Coord& xy) const {
    Log_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    Digit result = locked_getValue(xy);
    if (Log_Showing_Debug2) {
        Log_Debug2_T("result=" << static_cast<int>(result));
    }
    return result;
}


mdn::Digit mdn::Mdn2dBase::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        Log_Debug3("At " << xy << ": no entry, returning 0");
        return static_cast<Digit>(0);
    }
    if (Log_Showing_Debug3) {
        Log_Debug3("At " << xy << ": returning " << static_cast<int>(it->second));
    }
    return it->second;
}


std::vector<mdn::Digit> mdn::Mdn2dBase::getRow(int y) const {
    auto lock = lockReadOnly();
    Log_Debug2_H("");
    std::vector<Digit> result = locked_getRow(y);
    if (Log_Showing_Debug2) {
        Log_Debug2_T("Row " << y << ": " << Tools::digitArrayToString(result));
    }
    return result;
}



std::vector<mdn::Digit> mdn::Mdn2dBase::locked_getRow(int y) const {
    std::vector<Digit> digits;
    Log_Debug3_H("Row " << y);
    locked_getRow(y, digits);
    Log_Debug3_T("result=[array with " << digits.size() << " elements]");
    return digits;
}


void mdn::Mdn2dBase::getRow(int y, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    Log_Debug2_H("");
    locked_getRow(y, digits);
    if (Log_Showing_Debug2) {
        Log_Debug2_T("Row " << y << ": inplace" << Tools::digitArrayToString(digits));
    }
}


void mdn::Mdn2dBase::locked_getRow(int y, std::vector<Digit>& digits) const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x() + 1; // one past the end, prevent fencepost
    int xCount = xEnd - xStart;
    Log_Debug3_H(
        "Row " << y << " from x (" << xStart << " .. " << xEnd << "), " << xCount << " elements"
    );
    digits.resize(xCount);
    std::fill(digits.begin(), digits.end(), 0);
    auto it = m_yIndex.find(y);
    if (it != m_yIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            digits[coord.x()-xStart] = m_raw.at(coord);
        }
    }
    Log_Debug3_T("");
}


std::vector<mdn::Digit> mdn::Mdn2dBase::getCol(int x) const {
    auto lock = lockReadOnly();
    Log_Debug2_H("");
    std::vector<Digit> result = locked_getCol(x);
    if (Log_Showing_Debug2) {
        Log_Debug2_T("Column " << x << ": " << Tools::digitArrayToString(result));
    }
    return result;
}


std::vector<mdn::Digit> mdn::Mdn2dBase::locked_getCol(int x) const {
    std::vector<Digit> digits;
    Log_Debug3_H("Column " << x);
    locked_getCol(x, digits);
    Log_Debug3_T("result=[array with " << digits.size() << " elements]");
    return digits;
}


void mdn::Mdn2dBase::getCol(int x, std::vector<Digit>& digits) const {
    auto lock = lockReadOnly();
    Log_Debug2_H("Column " << x);
    locked_getCol(x, digits);
    if (Log_Showing_Debug2) {
        Log_Debug2_T("Column " << x << ": inplace" << Tools::digitArrayToString(digits));
    }
}


void mdn::Mdn2dBase::locked_getCol(int x, std::vector<Digit>& digits) const {
    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y() + 1; // one past the end, prevent fencepost
    int yCount = yEnd - yStart;
    Log_Debug3(
        "Column " << x << " from y (" << yStart << " .. " << yEnd << "), " << yCount << " elements"
    );
    digits.resize(yCount);
    std::fill(digits.begin(), digits.end(), 0);
    auto it = m_xIndex.find(x);
    if (it != m_xIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            digits[coord.x()-yStart] = m_raw.at(coord);
        }
    }
}


void mdn::Mdn2dBase::clear() {
    auto lock = lockWriteable();
    Log_Debug2_H("");
    locked_clear();
    internal_operationComplete();
    Log_Debug2_T("");
}


void mdn::Mdn2dBase::locked_clear() {
    Log_Debug3_H("");
    m_raw.clear();
    internal_clearMetadata();
    Log_Debug3_T("");
}


bool mdn::Mdn2dBase::setToZero(const Coord& xy) {
    auto lock = lockWriteable();
    Log_Debug2_H("Setting " << xy << " to zero");
    bool possibleCarryOver = locked_setToZero(xy);
    internal_operationComplete();
    Log_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::locked_setToZero(const Coord& xy) {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // Already zero
        Log_Debug3_H("Setting " << xy << " to zero: already zero");
        bool result = locked_checkPrecisionWindow(xy) != PrecisionStatus::Below;
        Log_Debug3_T("result=" << result);
        return result;
    }
    // There is currently a non-zero value - erase it
    if (Log_Showing_Debug3) {
        Log_Debug3_H(
            "Setting " << xy << " to zero: current value=" << static_cast<int>(it->second)
        );
    }
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
    internal_modified();

    CoordSet& coordsAlongX(xit->second);
    CoordSet& coordsAlongY(yit->second);
    coordsAlongX.erase(xy);
    coordsAlongY.erase(xy);
    m_index.erase(xy);
    bool checkBounds = false;
    if (coordsAlongX.size() == 0) {
        Log_Debug3("Erasing empty indexing column at " << xy.x());
        m_xIndex.erase(xit);
        checkBounds = true;
    }

    if (coordsAlongY.size() == 0) {
        Log_Debug3("Erasing empty indexing row at " << xy.y());
        m_yIndex.erase(yit);
        checkBounds = true;
    }
    if (checkBounds) {
        // Bounds may have changed
        Log_Debug3("Updating bounds");
        internal_updateBounds();
    }
    Log_Debug3_T("result=1");
    return true;
}


mdn::CoordSet mdn::Mdn2dBase::setToZero(const CoordSet& coords) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug3) {
        std::string coordsList(Tools::setToString<Coord>(coords, ','));
        Log_Debug3_H("Zeroing coord set: " << coordsList);
    } else {
        Log_Debug2_H("Zeroing set containing " << coords.size() << " coords");
    }
    CoordSet changed = locked_setToZero(coords);
    internal_operationComplete();
    if (Log_Showing_Debug3) {
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_Debug3_T("Returning changed coords: " << coordsList);
    } else {
        Log_Debug2_T("Returning set containing " << changed.size() << " changed coords");
    }
    return changed;
}


mdn::CoordSet mdn::Mdn2dBase::locked_setToZero(const CoordSet& purgeSet) {
    Log_Debug3_H("Zeroing set containing " << purgeSet.size() << " coords");

    CoordSet changed;
    // Step 1: Erase from m_raw
    for (const Coord& coord : purgeSet) {
        if (m_raw.erase(coord)) {
            changed.insert(coord);
        }
    }

    // Step 2: Clean up m_xIndex and m_yIndex
    int indexRowsRemoved = 0;
    int indexColsRemoved = 0;
    for (const Coord& coord : purgeSet) {
        int x = coord.x();
        int y = coord.y();

        // Erase coord from index
        m_index.erase(coord);

        // Erase coord from x index
        auto xIt = m_xIndex.find(x);
        if (xIt != m_xIndex.end()) {
            xIt->second.erase(coord);
            if (xIt->second.empty()) {
                m_xIndex.erase(xIt);
                ++indexColsRemoved;
            }
        }

        // Erase coord from y index
        auto yIt = m_yIndex.find(y);
        if (yIt != m_yIndex.end()) {
            yIt->second.erase(coord);
            if (yIt->second.empty()) {
                m_yIndex.erase(yIt);
                ++indexRowsRemoved;
            }
        }
    }
    if (indexRowsRemoved + indexColsRemoved > 0) {
        internal_modified();

        // Bounds may have changed
        internal_updateBounds();
    }
    Log_Debug3_T(
        "Erased " << indexRowsRemoved << " index rows and " << indexColsRemoved << " index cols"
    );

    return changed;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, Digit value) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug2) {
        Log_Debug2_H("Setting " << xy << " to " << static_cast<int>(value));
    }
    bool possibleCarryOver = locked_setValue(xy, value);
    internal_modifiedAndComplete();
    Log_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, int value) {
    auto lock = lockWriteable();
    Log_Debug2_H("Setting " << xy << " to " << value);
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long value) {
    auto lock = lockWriteable();
    Log_Debug2_H("Setting " << xy << " to " << value);
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long long value) {
    auto lock = lockWriteable();
    Log_Debug2_H("Setting " << xy << " to " << value);
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, Digit value) {
    Log_Debug3_H("Setting " << xy << " to " << static_cast<int>(value));
    internal_checkDigit(xy, value);
    bool result = internal_setValueRaw(xy, value);
    Log_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, int value) {
    Log_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, long value) {
    Log_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, long long value) {
    Log_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_Debug3_T("result=" << result);
    return result;
}


std::string mdn::Mdn2dBase::toString() const {
    auto lock = lockReadOnly();
    Log_Debug2_H("Converting to string");
    std::string result = locked_toString();
    Log_Debug2_T("result=" << result);
    return result;
}


std::string mdn::Mdn2dBase::locked_toString() const {
    Log_Debug3_H("Converting to string");
    std::vector<std::string> rows = locked_toStringRows();
    std::string result = Tools::vectorToString(rows, "\n", true);
    Log_Debug3_T("result=" << result);
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::toStringRows() const {
    auto lock = lockReadOnly();
    Log_Debug2_H("Converting rows to string array");
    std::vector<std::string> result = locked_toStringRows();
    Log_Debug2_T("result is a set of " << result.size() << " rows");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::locked_toStringRows() const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x()+1;
    int xCount = xEnd - xStart;

    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y()+1;
    int yCount = yEnd - yStart;

    Log_Debug3(
        "Converting rows to an array of strings:\n"
        << "    xRange = (" << xStart << " .. " << xEnd << "), " << xCount << " elements,"
        << "    yRange = (" << yStart << " .. " << yEnd << "), " << yCount << " rows,"
    );

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

        assert(digits.size() == xCount && "Rows are not the expected size");
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
    Log_Debug2("Converting columns to string array");
    return locked_toStringCols();
}


std::vector<std::string> mdn::Mdn2dBase::locked_toStringCols() const {
    int xStart = m_boundsMin.x();
    int xEnd = m_boundsMax.x()+1;
    int xCount = xEnd - xStart;

    int yStart = m_boundsMin.y();
    int yEnd = m_boundsMax.y()+1;
    int yCount = yEnd - yStart;

    Log_Debug3(
        "Converting columns to an array of strings:\n"
        << "    xRange = (" << xStart << " .. " << xEnd << "), " << xCount << " columns,"
        << "    yRange = (" << yStart << " .. " << yEnd << "), " << yCount << " elements"
    );

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
        assert(digits.size() == yCount && "Columns are not the expected size");
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
    Log_Debug2("");
    locked_rebuildMetadata();
}


void mdn::Mdn2dBase::locked_rebuildMetadata() const {
    Log_Debug3("");
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
    bool result = locked_hasBounds();
    Log_Debug2("Result: " << result);
    return result;
}


bool mdn::Mdn2dBase::locked_hasBounds() const {
    bool invalid = (
        m_boundsMin.x() == constants::intMax ||
        m_boundsMin.y() == constants::intMax ||
        m_boundsMax.x() == constants::intMin ||
        m_boundsMax.y() == constants::intMin
    );

    Log_Debug3(
        "Bounds:  min: " << m_boundsMin << "  max: " << m_boundsMax << "  result: " << (!invalid)
    );
    return !invalid;
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2dBase::getBounds() const {
    auto lock = ReadOnlyLock();
    std::pair<Coord, Coord> bounds = locked_getBounds();
    if (Log_Showing_Debug2) {
        Log_Debug2("Result: " << bounds.first << ", " << bounds.second);
    }
    return bounds;
}


std::pair<mdn::Coord, mdn::Coord> mdn::Mdn2dBase::locked_getBounds() const {
    std::pair<Coord, Coord> bounds(m_boundsMin, m_boundsMax);
    Log_Debug3("Result: " << bounds.first << ", " << bounds.second);
    return bounds;
}


int mdn::Mdn2dBase::getPrecision() const {
    auto lock = lockReadOnly();
    Log_Debug2_H("");
    int result = locked_getPrecision();
    Log_Debug2_T("result=" << result);
    return result;
}


int mdn::Mdn2dBase::locked_getPrecision() const {
    int result = m_config.precision();
    Log_Debug3("Result: " << result);
    return result;
}


int mdn::Mdn2dBase::setPrecision(int newPrecision) {
    auto lock = lockWriteable();
    Log_Debug2_H("New precision: " << newPrecision);
    int nDropped = locked_setPrecision(newPrecision);
    internal_operationComplete();
    Log_Debug2_T("result=" << nDropped);
    return nDropped;
}


int mdn::Mdn2dBase::locked_setPrecision(int newPrecision) {
    int oldPrecision = m_config.precision();
    m_config.setPrecision(newPrecision);
    Log_Debug3("Precision was: " << oldPrecision << ", new value: " << newPrecision);

    if (oldPrecision < newPrecision)
    {
        return internal_purgeExcessDigits();
    }
    return 0;
}


mdn::PrecisionStatus mdn::Mdn2dBase::checkPrecisionWindow(const Coord& xy) const {
    auto lock = ReadOnlyLock();
    Log_Debug3_H("At: " << xy);
    PrecisionStatus result = locked_checkPrecisionWindow(xy);
    if (Log_Showing_Debug3) {
        Log_Debug3_T("result=" << PrecisionStatusToName(result));
    }
    return result;
}


mdn::PrecisionStatus mdn::Mdn2dBase::locked_checkPrecisionWindow(const Coord& xy) const {
    if (!locked_hasBounds()) {
        Log_Debug4("At: " << xy << ", result: Inside (no bounds)");
        return PrecisionStatus::Inside;
    }
    int precision = m_config.precision();

    // minLimit - below this and the new value should not be added
    Coord minLimit = m_boundsMax - precision;

    // Check under limit
    if (xy.x() < minLimit.x() || xy.y() < minLimit.y()) {
        // Value is under-limit, do not add
        Log_Debug4("At: " << xy << ", result: under limit " << minLimit);
        return PrecisionStatus::Below;
    }

    // Check over limit
    CoordSet purgeSet;
    // maxLimit - above this and we need to purge existing values
    Coord maxLimit = m_boundsMin + precision;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    int purgeY = xy.y() - maxLimit.y();
    if (purgeX > 0 || purgeY > 0) {
        Log_Debug4("At: " << xy << ", result: Above (digits need to be purged)");
        return PrecisionStatus::Above;
    }
    Log_Debug4("At: " << xy << ", result: Inside");
    return PrecisionStatus::Inside;
}


void mdn::Mdn2dBase::internal_modified() {
    Log_Debug4("Modified flag set");
    m_modified = true;
}


void mdn::Mdn2dBase::internal_operationComplete() {
    if (m_modified) {
        Log_Debug4("Operation complete, incrementing m_event from " << m_event);
        ++m_event;
        m_modified = false;
    } else {
        Log_Debug4("Operation complete, no modifications");
    }
}


void mdn::Mdn2dBase::internal_modifiedAndComplete() {
    Log_Debug4("Operation complete and modified, incrementing m_event from " << m_event);
    ++m_event;
    m_modified = false;
}


mdn::Mdn2dBase::WritableLock mdn::Mdn2dBase::lockWriteable() const {
    Log_Debug4("Waiting for WRITE lock");
    return std::unique_lock(m_mutex);
}


mdn::Mdn2dBase::ReadOnlyLock mdn::Mdn2dBase::lockReadOnly() const {
    Log_Debug4("Waiting for READ lock");
    return std::shared_lock(m_mutex);
}


void mdn::Mdn2dBase::assertNotSelf(Mdn2dBase& that, const std::string& description) const {
    if (this == &that) {
        IllegalSelfReference err(description);
        Log_Error(err.what());
        throw err;
    }
    Log_Debug4("Operation okay, not self");
}


void mdn::Mdn2dBase::internal_clearMetadata() const {
    Log_Debug3("");
    m_boundsMin = Coord({constants::intMax, constants::intMax});
    m_boundsMax = Coord({constants::intMin, constants::intMin});

    m_xIndex.clear();
    m_yIndex.clear();
    m_index.clear();
}


bool mdn::Mdn2dBase::internal_setValueRaw(const Coord& xy, Digit value) {
    if (value == 0) {
        if (Log_Showing_Debug4) {
            Log_Debug4_H(
                "Setting " << xy << " to " << static_cast<int>(value) << ": setting to zero"
            );
        }
        locked_setToZero(xy);
        Log_Debug4_T("result=0");
        return false;
    }

    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // No entry exists
        if (Log_Showing_Debug4) {
            Log_Debug4_H(
                "Setting " << xy << " to " << static_cast<int>(value)
                << ", no previous existing value"
            );
        }
        PrecisionStatus ps = locked_checkPrecisionWindow(xy);
        if (ps == PrecisionStatus::Below) {
            // Out of numerical precision range
            Log_Debug4_T("New value below precision range, result=0");
            return false;
        }
        internal_modified();
        internal_insertAddress(xy);
        m_raw[xy] = value;
        if (ps == PrecisionStatus::Above) {
            // Above numerical precision range
            Log_Debug4("New value above precision range, purging low digits");
            internal_purgeExcessDigits();
            Log_Debug4_T("result=1");
            return true;
        }
        if (Log_Showing_Debug4) {
            Log_Debug4_T("New value within precision range, result=1");
        }
        return true;
    }
    // xy is already non-zero
    Digit oldVal = it->second;
    it->second = value;
    if (oldVal != value) {
        internal_modified();
    } else if (Log_Showing_Debug4) {
        Log_Debug4(
            "Setting " << xy << " to " << static_cast<int>(value)
            << ", but digit already has that value, result=0"
        );
        return false;
    }

    if (Log_Showing_Debug4) {
        Log_Debug4_H(
            "Setting " << xy << " to " << static_cast<int>(value)
            << ": overwriting existing value: " << static_cast<int>(oldVal)
        );
    }
    // check for sign change
    bool result = (
        (oldVal < 0 && value > 0) || (oldVal > 0 && value < 0)
    );
    Log_Debug4_T("result=" << result);
    return result;
    // Possibly faster
    // return (static_cast<int>(oldVal) * static_cast<int>(value)) < 0;
}


void mdn::Mdn2dBase::internal_insertAddress(const Coord& xy) const {
    Log_Debug4("At: " << xy);
    m_index.insert(xy);
    auto xit = m_xIndex.find(xy.x());
    if (xit == m_xIndex.end()) {
        m_xIndex.emplace(xy.x(), CoordSet());
        CoordSet newX;
        newX.insert(xy);
        m_xIndex[xy.x()] = newX;
    } else {
        xit->second.insert(xy);
    }
    auto yit = m_yIndex.find(xy.y());
    if (yit == m_yIndex.end()) {
        m_yIndex.emplace(xy.y(), CoordSet());
        CoordSet newy;
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


int mdn::Mdn2dBase::internal_purgeExcessDigits() {
    if (!locked_hasBounds()) {
        // No digits to bother keeping
        Log_Debug3("No digits to consider");
        return 0;
    }

    int precision = m_config.precision();
    Coord currentSpan = m_boundsMax - m_boundsMin;
    CoordSet purgeSet;
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
        if (Log_Showing_Debug) {
            Log_Debug_H(
                "Purging " << purgeSet.size() << " digits, now below numerical precision window: "
                << "  min: " << m_boundsMin
                << "  max: " << m_boundsMax
                << "  precision: " << m_config.precision()
            );
        }
        locked_setToZero(purgeSet);
        Log_Debug_T("result=" << purgeSet.size());
        return purgeSet.size();
    }
    Log_Debug3("No digits purged");
    return 0;
}


void mdn::Mdn2dBase::internal_updateBounds() {
    if (m_xIndex.empty() || m_yIndex.empty()) {
        m_boundsMin = Coord({constants::intMax, constants::intMax});
        m_boundsMax = Coord({constants::intMin, constants::intMin});
        Log_Debug3("Updating bounds: no non-zero digits exist, there are no bounds");
    } else {
        auto itMinX = m_xIndex.cbegin();
        auto itMaxX = m_xIndex.crbegin();
        auto itMinY = m_yIndex.cbegin();
        auto itMaxY = m_yIndex.crbegin();
        m_boundsMin = {itMinX->first, itMinY->first};
        m_boundsMax = {itMaxX->first, itMaxY->first};
        Log_Debug3(
            "Updating bounds, new bounds: "
            << "   min: " << m_boundsMin
            << "   max: " << m_boundsMax
        );
    }
}
