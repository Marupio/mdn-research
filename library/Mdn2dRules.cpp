#include "Mdn2dRules.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "Mdn2d.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Carryover mdn::Mdn2dRules::static_checkCarryover(Digit p, Digit x, Digit y, Digit base) {
    Carryover result = Carryover::Invalid;
    if (abs(p) > base) {
        result = Carryover::Required;
    } else {
        auto pos = [](Digit t) { return t > 0; };
        auto neg = [](Digit t) { return t < 0; };

        if (pos(p)) {
            if (neg(x) && neg(y)) {
                result = Carryover::Required;
            }
            else if (neg(x) || neg(y)) {
                result = Carryover::OptionalPositive;
            }
        } else if (neg(p)) {
            if (pos(x) && pos(y)) {
                result = Carryover::Required;
            } else if (pos(x) || pos(y)) {
                result = Carryover::OptionalNegative;
            }
        }
    }

    if (Log_Showing_Debug4) {
        Log_Debug4("Checking with base " << static_cast<int>(base) << " ("
            << static_cast<int>(p) << ":"
            << static_cast<int>(x) << ","
            << static_cast<int>(y) << "): result = "
            << CarryoverToName(result)
        );
    }
    return result;
}


mdn::Mdn2dRules::Mdn2dRules(std::string nameIn) :
    Mdn2dBase(nameIn)
{
    Log_Debug3("");
}


mdn::Mdn2dRules::Mdn2dRules(Mdn2dConfig config, std::string nameIn) :
    Mdn2dBase(config, nameIn)
{
    Log_Debug3("Config=" << config);
}


mdn::Mdn2dRules::Mdn2dRules(const Mdn2dRules& other, std::string nameIn):
    Mdn2dBase(other, nameIn)
{
    Log_Debug3("");
}


mdn::Mdn2dRules& mdn::Mdn2dRules::operator=(const Mdn2dRules& other) {
    Mdn2dBase::operator=(other);
    if (this != &other) {
        // Grab another lock, base's lock has expired
        Log_Debug3("Setting Mdn2dRules equal to other");
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();
        if (other.m_event == other.m_polymorphicNodes_event) {
            m_polymorphicNodes = other.m_polymorphicNodes;
            m_polymorphicNodes_event = m_event;
        }
    }
    return *this;
}


mdn::Mdn2dRules::Mdn2dRules(Mdn2dRules&& other, std::string nameIn) noexcept :
    Mdn2dBase(std::move(other), nameIn)
{
    auto lockOther = other.lockReadOnly();
    Log_Debug3("Move-copying Mdn2dRules");
    if (other.m_event == other.m_polymorphicNodes_event) {
        m_polymorphicNodes = std::move(other.m_polymorphicNodes);
        m_polymorphicNodes_event = m_event;
    }
}


mdn::Mdn2dRules& mdn::Mdn2dRules::operator=(Mdn2dRules&& other) noexcept {
    Mdn2dBase::operator=(std::move(other));
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();
        Log_Debug3("Setting Mdn2dRules equal to other, move-copy");
        if (other.m_event == other.m_polymorphicNodes_event) {
            m_polymorphicNodes = std::move(other.m_polymorphicNodes);
            m_polymorphicNodes_event = m_event;
        }
    }
    return *this;
}


void mdn::Mdn2dRules::locked_setConfig(Mdn2dConfig newConfig) {
    Log_N_Debug3_H("");
    SignConvention origSc = m_config.signConvention();
    SignConvention newSc = newConfig.signConvention();
    Mdn2dBase::locked_setConfig(newConfig);
    if (origSc != newSc) {
        Log_N_Debug4("SignConvention changed: checking for polymorphic carryovers.");
        int nChanged = locked_carryoverCleanupAll().size();
        Log_N_Debug3("SignConvention change led to " << nChanged << " polymorphic changes.");
    }
    Log_N_Debug3_T("");
}


mdn::Carryover mdn::Mdn2dRules::checkCarryover(const Coord& xy) const {
    auto lock = lockReadOnly();
    Log_N_Debug2_H("At: " << xy);
    Carryover result = locked_checkCarryover(xy);
    if (Log_Showing_Debug2) {
        Log_N_Debug2_T("result=" << CarryoverToName(result));
    }
    return result;
}


mdn::Carryover mdn::Mdn2dRules::locked_checkCarryover(const Coord& xy) const {
    Log_N_Debug3_H("At " << xy);
    Carryover result = static_checkCarryover(
        locked_getValue(xy),
        locked_getValue(xy.translatedX(1)),
        locked_getValue(xy.translatedY(1)),
        m_config.baseDigit()
    );
    if (Log_Showing_Debug3) {
        Log_N_Debug3_T("result=" << CarryoverToName(result));
    }
    return result;
}


mdn::CoordSet mdn::Mdn2dRules::carryover(const Coord& xy) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("At: " << xy);
    CoordSet changed = locked_carryover(xy);
    internal_operationComplete();
    if (Log_Showing_Debug4) {
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug4("changed=" << coordsList);
    }
    Log_N_Debug2_T("result=[set of coords with " << changed.size() << " elements]");
    return changed;
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryover(const Coord& xy, int carry) {
    Log_N_Debug3_H("At " << xy << ", carry=" << carry);
    Coord xy_x = xy.translatedX(1);
    Coord xy_y = xy.translatedY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    Carryover co = static_checkCarryover(p, x, y, m_config.baseDigit());
    if (co == Carryover::Invalid) {
        IllegalOperation err("Invalid carryover requested at " + xy.to_string());
        Log_N_Error(err.what());
        throw err;
    }
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry;
    if (carry == 0) {
        // The first carryover, an optional carry
        if (ip < 0) {
            nCarry = -1;
        } else {
            nCarry = 1;
        }
    } else {
        nCarry = (ip + carry)/m_config.base();
    }
    #ifdef MDN_DEBUG
        if (nCarry > 1 || nCarry < -1) {
            std::ostringstream oss;
            oss << "Internal warning: carryover at " << xy << ": magnitude should not exceed 1. ";
            oss << "Got " << nCarry << "." << std::endl;
            Log_N_Warn(oss.str());
        }
    #endif
    ip -= nCarry * m_config.base();
    iy += nCarry;
    ix += nCarry;

    int base(m_config.base());
    CoordSet affectedCoords({xy, xy_x, xy_y});
    locked_setValue(xy, ip);

    if (Log_Showing_Debug3) {
        // Examples:
        //  * Carryover: (-4:3,-2) ==> (6:2,-3)
        //  * Carryover: (6:9,0) ==> (-4:10*,-2)
        //  * Carryover: (-2:9,-9) ==> (8:8,-10*)
        std::string xStar = (ix >= base || ix <= -base) ? "*" : "";
        std::string yStar = (iy >= base || iy <= -base) ? "*" : "";
        Log_N_Debug3(
            "Carrover: " << nCarry << "(+" << carry << "):("
            << static_cast<int>(p) << ":"
            << static_cast<int>(x) << ","
            << static_cast<int>(y) << ") ==> "
            << "(" << ip << ":" << ix << xStar << "," << iy << yStar << ")"
        );
    }

    // Check for additional carryovers
    if (iy >= base || iy <= -base) {
        Log_N_Debug4("cascading carryover to " << xy_y << " adding " << nCarry);
        affectedCoords.merge(locked_carryover(xy_y, nCarry));
    } else {
        Log_N_Debug4("no further carryovers in y, at " << xy_y << " setting to " << iy);
        locked_setValue(xy_y, iy);
    }
    if (ix >= base || ix <= -base) {
        Log_N_Debug4("cascading carryover to " << xy_x << " adding " << nCarry);
        affectedCoords.merge(locked_carryover(xy_x, nCarry));
    } else {
        Log_N_Debug4("no further carryovers in y, at " << xy_x << " setting to " << iy);
        locked_setValue(xy_x, ix);
    }
    affectedCoords.merge(locked_carryoverCleanup(affectedCoords));
    if (Log_Showing_Debug4) {
        std::string coordsList(Tools::setToString<Coord>(affectedCoords, ','));
        Log_N_Debug4("affectedCoords=" << coordsList);
    }
    Log_N_Debug3_T("result=[set of coords with " << affectedCoords.size() << " elements]");
    return affectedCoords;
}


mdn::CoordSet mdn::Mdn2dRules::carryoverCleanup(const CoordSet& coords) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("Carryover clean up on " << coords.size() << " coords");
    CoordSet changed = locked_carryoverCleanup(coords);
    // if (coords.size()) {
    // }
    internal_operationComplete();
    if (Log_Showing_Debug4) {
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug4("changed=" << coordsList);
    }
    Log_N_Debug2_T("result=[set of coords with " << changed.size() << " elements]");
    return changed;
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryoverCleanup(const CoordSet& coords) {
    CoordSet affectedCoords;
    if (coords.empty()) {
        Log_N_Debug3("Carryover cleanup, no coords to check");
        return affectedCoords;
    }
    CoordSet buffer;

    Log_N_Debug3_H("Input set of " << coords.size() << " elements for carryoverCleanup");
    if (Log_Showing_Debug4) {
        std::string coordsList(Tools::setToString<Coord>(coords, ','));
        Log_N_Debug4("coords=" << coordsList);
    }
    Carryover wrongSign = Carryover::Required;
    if (m_config.signConvention() == SignConvention::Positive) {
        wrongSign = Carryover::OptionalNegative;
    } else if (m_config.signConvention() == SignConvention::Negative) {
        wrongSign = Carryover::OptionalPositive;
    }

    CoordSet workingSet(coords);
    bool achievedGreatness = false;
    for (int i = 0; i < m_config.maxCarryoverIters(); ++i) {
        for (const Coord& xy: workingSet) {
            Carryover co = locked_checkCarryover(xy);
            if (co == Carryover::Required || co == wrongSign) {
                buffer.merge(locked_carryover(xy));
            }
        }
        workingSet = buffer;
        affectedCoords.merge(buffer);
        buffer.clear();
        if (!workingSet.size()) {
            achievedGreatness = true;
        }
    }
    if (!achievedGreatness) {
        Log_N_Warn(
            "Failed to finish all required carryovers and carryover sign " << "conventions.\n"
            << "\tMax iterations: " << m_config.maxCarryoverIters() << '\n'
            << "\tDigits remaining to check: " << workingSet.size() << '\n'
            << "\tTotal digits affected: " << affectedCoords.size() << '\n'
        );
        Log_N_Debug3_T("Carryover cleanup on " << coords.size() << " coords failed.");
    } else {
        Log_N_Debug3_T("Carryover cleanup on " << coords.size() << " coords complete.");
    }
    return affectedCoords;
}


mdn::CoordSet mdn::Mdn2dRules::carryoverCleanupAll() {
    auto lock = lockWriteable();
    Log_N_Debug2_H("");
    CoordSet changed = locked_carryoverCleanupAll();
    internal_operationComplete();
    Log_N_Debug2_T("result=[set of coords with " << changed.size() << " elements]");
    return changed;
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryoverCleanupAll() {
    Log_N_Debug4_H("");
    CoordSet changed = locked_carryoverCleanup(m_index);
    if (Log_Showing_Debug4) {
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug4("changed=" << coordsList);
    }
    return changed;
}


void mdn::Mdn2dRules::shift(int xDigits, int yDigits) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("shifting (" << xDigits << "," << yDigits << ")");
    locked_shift(xDigits, yDigits);
    internal_operationComplete();
    Log_N_Debug2_T("")
}


void mdn::Mdn2dRules::shift(const Coord& xy) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H("shifting (" << xy.x() << "," << xy.y() << ")");
    }
    locked_shift(xy);
    internal_operationComplete();
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::locked_shift(const Coord& xy) {
    if (Log_Showing_Debug3) {
        Log_N_Debug3_H("shifting (" << xy.x() << "," << xy.y() << ")");
    }
    locked_shift(xy.x(), xy.y());
    Log_N_Debug3_T("");
}
//TODO - Do we update metadata with shifts?  e.g. polymorphic coords?  Maybe we don't track
// keep track of polymorphic carryovers.

void mdn::Mdn2dRules::locked_shift(int xDigits, int yDigits) {
    Log_N_Debug3_H("shift (" << xDigits << "," << yDigits << ")");
    if (xDigits > 0) {
        Log_N_Debug3("shifting right (" << xDigits << ")");
        locked_shiftRight(xDigits);
        internal_modified();
    } else if (xDigits < 0) {
        Log_N_Debug3("shifting left (" << -xDigits << ")");
        locked_shiftLeft(-xDigits);
        internal_modified();
    }
    if (yDigits > 0) {
        Log_N_Debug3("shifting up (" << yDigits << ")");
        locked_shiftUp(yDigits);
        internal_modified();
    } else if (yDigits < 0) {
        Log_N_Debug3("shifting down (" << -yDigits << ")");
        locked_shiftDown(-yDigits);
        internal_modified();
    }
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::shiftRight(int nDigits) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("shifting right (" << nDigits << ")");
    locked_shiftRight(nDigits);
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dRules::locked_shiftRight(int nDigits) {
    Log_N_Debug3_H("shifting right " << nDigits);
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_xIndex.rbegin(); it != m_xIndex.rend(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.translatedX(nDigits)] = d;
        }
    }
    internal_modified();
    locked_rebuildMetadata();
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::shiftLeft(int nDigits) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("shifting left (" << nDigits << ")");
    locked_shiftLeft(nDigits);
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dRules::locked_shiftLeft(int nDigits) {
    Log_N_Debug3_H("shifting left (" << nDigits << ")");
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_xIndex.begin(); it != m_xIndex.end(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.translatedX(-nDigits)] = d;
        }
    }
    internal_modified();
    locked_rebuildMetadata();
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::shiftUp(int nDigits) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("shifting up (" << nDigits << ")");
    locked_shiftUp(nDigits);
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dRules::locked_shiftUp(int nDigits) {
    Log_N_Debug3_H("shifting right (" << nDigits << ")");
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_yIndex.rbegin(); it != m_yIndex.rend(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.translatedY(nDigits)] = d;
        }
    }
    internal_modified();
    locked_rebuildMetadata();
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::shiftDown(int nDigits) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("shifting down (" << nDigits << ")");
    locked_shiftDown(nDigits);
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dRules::locked_shiftDown(int nDigits) {
    Log_N_Debug3_H("shifting down (" << nDigits << ")");
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    for (auto it = m_yIndex.begin(); it != m_yIndex.end(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.translatedY(-nDigits)] = d;
        }
    }
    internal_modified();
    locked_rebuildMetadata();
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::transpose() {
    auto lock = lockWriteable();
    Log_N_Debug2_H("");
    locked_transpose();
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dRules::locked_transpose() {
    Log_N_Debug3_H("");
    Mdn2d temp(NewInstance(m_config));
    auto tempLock = temp.lockWriteable();
    for (const auto& [xy, digit] : m_raw) {
        temp.locked_setValue(Coord(xy.y(), xy.x()), digit);
    }
    operator=(temp);
    if (m_raw.size()) {
        internal_modified();
    }
    Log_N_Debug3_T("");
}


const mdn::CoordSet& mdn::Mdn2dRules::getPolymorphicNodes() const {
    auto lock = ReadOnlyLock();
    Log_N_Debug2_H("");
    const mdn::CoordSet& coords = locked_getPolymorphicNodes();
    Log_N_Debug2_T("");
    return coords;
}


const mdn::CoordSet& mdn::Mdn2dRules::locked_getPolymorphicNodes() const {
    Log_N_Debug3_H("");
    if (m_polymorphicNodes_event != m_event) {
        Log_N_Debug3(
            "polymorphicNodes out-of-date: (data: " << m_polymorphicNodes_event
            << ", current: " << m_event << "), updating..."
        );
        internal_polymorphicScan();
    } else {
            Log_N_Debug3("already up-to-date, returning");
    }
    Log_N_Debug3_T("");
    return m_polymorphicNodes;
}


void mdn::Mdn2dRules::internal_polymorphicScan() const {
    Log_N_Debug3_H("");
    int nRequired = 0;
    m_polymorphicNodes.clear();
    for (const auto& [xy, digit] : m_raw) {
        switch(locked_checkCarryover(xy)) {
            case Carryover::Required:
                ++nRequired;
                break;
            case Carryover::OptionalPositive:
            case Carryover::OptionalNegative:
                m_polymorphicNodes.insert(xy);
                break;
            default:
                break;
        }
    }
    if (nRequired) {
        Log_N_Warn(
            "Internal error: found " << nRequired << " required carryovers during scan.\n"
            << "MDN is in an invalid state."
        );
    }
    m_polymorphicNodes_event = m_event;
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::internal_oneCarryover(const Coord& xy) {
    Log_N_Debug4_H("At " << xy);
    Coord xy_x = xy.translatedX(1);
    Coord xy_y = xy.translatedY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_config.base();
    Log_N_Debug4("Before carry (" << ip << ":" << ix << "," << iy << "): carry=" << nCarry);
    if (ip > 0) {
        ip -= m_config.base();
        iy += 1;
        ix += 1;
    } else {
        ip += m_config.base();
        iy -= 1;
        ix -= 1;
    }
    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
    Log_N_Debug4_T("After carry (" << ip << ":" << ix << "," << iy << ")");
}


void mdn::Mdn2dRules::internal_ncarryover(const Coord& xy) {
    Log_N_Debug3_H("");
    Coord xy_x = xy.translatedX(1);
    Coord xy_y = xy.translatedY(1);
    Digit p = locked_getValue(xy);
    #ifdef MDN_DEBUG
        if (abs(p) < m_config.baseDigit()) {
            std::ostringstream oss;
            oss << "Invalid carryover requested at " << xy << ": value (" << p << ") must exceed "
                << "base (" << m_config.baseDigit() << ")";
            throw IllegalOperation(oss.str());
        }
    #endif
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_config.base();
    ip -= nCarry * m_config.base();
    iy += nCarry;
    ix += nCarry;
    if (Log_Showing_Debug4) {
        Log_N_Debug4("Carryover with base " << static_cast<int>(m_config.base()) << " ("
            << static_cast<int>(p) << ":"
            << static_cast<int>(x) << ","
            << static_cast<int>(y) << "): result = ("
            << ip << ":" << ix << "," << iy << ")"
        );
    }
    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
    Log_N_Debug3_T("");
}


void mdn::Mdn2dRules::internal_clearMetadata() const {
    Log_N_Debug3_H("");
    m_polymorphicNodes.clear();
    m_polymorphicNodes_event = -1;
    Mdn2dBase::internal_clearMetadata();
    Log_N_Debug3_T("");
}
