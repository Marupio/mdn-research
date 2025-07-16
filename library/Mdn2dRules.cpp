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


mdn::Mdn2dRules::Mdn2dRules() :
    Mdn2dBase()
{
    Log_Debug3("Constructing null Mdn2dRules");
}


mdn::Mdn2dRules::Mdn2dRules(Mdn2dConfig config) :
    Mdn2dBase(config)
{
    Log_Debug3("Constructing Mdn2dRules with config " << config);
}


mdn::Mdn2dRules::Mdn2dRules(const Mdn2dRules& other):
    Mdn2dBase(other)
{
    Log_Debug3("Constructing Mdn2dRules as copy");
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


mdn::Mdn2dRules::Mdn2dRules(Mdn2dRules&& other) noexcept :
    Mdn2dBase(std::move(other))
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


mdn::Carryover mdn::Mdn2dRules::checkCarryover(const Coord& xy) const {
    auto lock = lockReadOnly();
    Log_Debug2("At: " << xy);
    return locked_checkCarryover(xy);
}


mdn::Carryover mdn::Mdn2dRules::locked_checkCarryover(const Coord& xy) const {
    return static_checkCarryover(
        locked_getValue(xy),
        locked_getValue(xy.copyTranslateX(1)),
        locked_getValue(xy.copyTranslateY(1)),
        m_config.dbase()
    );
}


mdn::CoordSet mdn::Mdn2dRules::carryover(const Coord& xy) {
    auto lock = lockWriteable();
    Log_Debug2("At: " << xy);
    modified();
    return locked_carryover(xy);
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryover(const Coord& xy, int carry) {
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    Carryover co = static_checkCarryover(p, x, y, m_config.dbase());
    if (co == Carryover::Invalid) {
        IllegalOperation err("Invalid carryover requested at " + xy.to_string());
        Log_Error(err.what());
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
        nCarry = ip/m_config.base();
    }
    #ifdef MDN_DEBUG
        if (nCarry > 1 || nCarry < -1) {
            std::ostringstream oss;
            oss << "Internal warning: carryover at " << xy << ": magnitude should not exceed 1. ";
            oss << "Got " << nCarry << "." << std::endl;
            Log_Warn(oss.str());
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
        std::string xStar = (ix > base || ix < -base) ? "*" : "";
        std::string yStar = (iy > base || iy < -base) ? "*" : "";
        Log_Debug3(
            "Carrover: " << nCarry << "(+" << carry << "):("
            << static_cast<int>(p) << ":"
            << static_cast<int>(x) << ","
            << static_cast<int>(y) << ") ==> "
            << "(" << ip << ":" << ix << xStar << "," << iy << yStar << ")"
        );
    }

    // Check for additional carryovers
    if (iy > base || iy < -base) {
        affectedCoords.merge(locked_carryover(xy_y, nCarry));
    } else {
        locked_setValue(xy_x, ix);
    }
    if (ix > base || ix < -base) {
        affectedCoords.merge(locked_carryover(xy_x, nCarry));
    } else {
        locked_setValue(xy_y, iy);
    }
    affectedCoords.merge(locked_carryoverCleanup(affectedCoords));
    return affectedCoords;
}


mdn::CoordSet mdn::Mdn2dRules::carryoverCleanup(const CoordSet& coords) {
    auto lock = lockWriteable();
    Log_Debug2("Carryover clean up on " << coords.size() << " coords");
    CoordSet changed = locked_carryoverCleanup(coords);
    if (coords.size()) {
        modified();
    }
    return changed;
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryoverCleanup(const CoordSet& coords) {
    CoordSet affectedCoords;
    if (coords.empty()) {
        Log_Debug3("Carryover cleanup, no coords to check");
        return affectedCoords;
    }
    CoordSet buffer;

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
        Log_Warn(
            "Failed to finish all required carryovers and carryover sign " << "conventions.\n"
            << "\tMax iterations: " << m_config.maxCarryoverIters() << '\n'
            << "\tDigits remaining to check: " << workingSet.size() << '\n'
            << "\tTotal digits affected: " << affectedCoords.size() << '\n'
        );
    } else {
        Log_Debug3("Carryover cleanup on " << coords.size() << " coords complete.");
    }
    return affectedCoords;
}


mdn::CoordSet mdn::Mdn2dRules::carryoverCleanupAll() {
    auto lock = lockWriteable();
    Log_Debug2("");
    return locked_carryoverCleanupAll();
}


mdn::CoordSet mdn::Mdn2dRules::locked_carryoverCleanupAll() {
    Log_Debug4("");
    return locked_carryoverCleanup(m_index);
}


void mdn::Mdn2dRules::shift(int xDigits, int yDigits) {
    auto lock = lockWriteable();
    Log_Debug2("shifting (" << xDigits << "," << yDigits << ")");
    modified();
    locked_shift(xDigits, yDigits);
}


void mdn::Mdn2dRules::shift(const Coord& xy) {
    auto lock = lockWriteable();
    if (Log_Showing_Debug3) {
        Log_Debug2("shifting (" << xy.x() << "," << xy.y() << ")");
    }
    modified();
    locked_shift(xy);
}


void mdn::Mdn2dRules::locked_shift(const Coord& xy) {
    if (Log_Showing_Debug3) {
        Log_Debug3("shifting (" << xy.x() << "," << xy.y() << ")");
    }
    locked_shift(xy.x(), xy.y());
}
//TODO - Do we update metadata with shifts?  e.g. polymorphic coords?  Maybe we don't track
// keep track of polymorphic carryovers.

void mdn::Mdn2dRules::locked_shift(int xDigits, int yDigits) {
    if (xDigits > 0) {
        Log_Debug3("shifting right (" << xDigits << ")");
        locked_shiftRight(xDigits);
    } else if (xDigits < 0) {
        Log_Debug3("shifting left (" << -xDigits << ")");
        locked_shiftLeft(-xDigits);
    }
    if (yDigits > 0) {
        Log_Debug3("shifting up (" << yDigits << ")");
        locked_shiftUp(yDigits);
    } else if (yDigits < 0) {
        Log_Debug3("shifting down (" << -yDigits << ")");
        locked_shiftDown(-yDigits);
    }
}


void mdn::Mdn2dRules::shiftRight(int nDigits) {
    auto lock = lockWriteable();
    Log_Debug2("shifting right (" << nDigits << ")");
    modified();
    locked_shiftRight(nDigits);
}


void mdn::Mdn2dRules::locked_shiftRight(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    Log_Debug3("shifting right " << nDigits);
    for (auto it = m_xIndex.rbegin(); it != m_xIndex.rend(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateX(nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2dRules::shiftLeft(int nDigits) {
    auto lock = lockWriteable();
    Log_Debug2("shifting left (" << nDigits << ")");
    modified();
    locked_shiftLeft(nDigits);
}


void mdn::Mdn2dRules::locked_shiftLeft(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    Log_Debug3("shifting left (" << nDigits << ")");
    for (auto it = m_xIndex.begin(); it != m_xIndex.end(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateX(-nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2dRules::shiftUp(int nDigits) {
    auto lock = lockWriteable();
    Log_Debug2("shifting up (" << nDigits << ")");
    modified();
    locked_shiftUp(nDigits);
}


void mdn::Mdn2dRules::locked_shiftUp(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    Log_Debug3("shifting right (" << nDigits << ")");
    for (auto it = m_yIndex.rbegin(); it != m_yIndex.rend(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateY(nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2dRules::shiftDown(int nDigits) {
    auto lock = lockWriteable();
    Log_Debug2("shifting down (" << nDigits << ")");
    modified();
    locked_shiftDown(nDigits);
}


void mdn::Mdn2dRules::locked_shiftDown(int nDigits) {
    #ifdef MDN_DEBUG
        if (nDigits < 0) {
            throw std::invalid_argument("cannot shift negative digits, use opposite direction");
        }
    #endif
    Log_Debug3("shifting down (" << nDigits << ")");
    for (auto it = m_yIndex.begin(); it != m_yIndex.end(); ++it) {
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            Digit d = m_raw[coord];
            m_raw.erase(coord);
            m_raw[coord.copyTranslateY(-nDigits)] = d;
        }
    }
    locked_rebuildMetadata();
}


void mdn::Mdn2dRules::transpose() {
    auto lock = lockWriteable();
    Log_Debug2("");
    modified();
    locked_transpose();
}


void mdn::Mdn2dRules::locked_transpose() {
    Log_Debug3("");
    Mdn2d temp(NewInstance(m_config));
    auto tempLock = temp.lockWriteable();
    for (const auto& [xy, digit] : m_raw) {
        temp.locked_setValue(Coord(xy.y(), xy.x()), digit);
    }
    operator=(temp);
}


const mdn::CoordSet& mdn::Mdn2dRules::getPolymorphicNodes() const {
    auto lock = ReadOnlyLock();
    Log_Debug2("");
    return locked_getPolymorphicNodes();
}


const mdn::CoordSet& mdn::Mdn2dRules::locked_getPolymorphicNodes() const {
    if (m_polymorphicNodes_event != m_event) {
        Log_Debug3(
            "polymorphicNodes out-of-date: (data: " << m_polymorphicNodes_event
            << ", current: " << m_event << "), updating..."
        );
        internal_polymorphicScan();
    } else {
            Log_Debug3("already up-to-date, returning");
    }
    return m_polymorphicNodes;
}


void mdn::Mdn2dRules::internal_polymorphicScan() const {
    Log_Debug3("");
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
        std::ostringstream oss;
        oss << "Internal error: found " << nRequired << " required carryovers during scan."
            << std::endl;
        oss << "MDN is in an invalid state." << std::endl;
        Logger::instance().warn(oss.str());
    }
    m_polymorphicNodes_event = m_event;
}


void mdn::Mdn2dRules::internal_oneCarryover(const Coord& xy) {
    Log_Debug4("");
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    int ip = static_cast<int>(p);
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_config.base();
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
}


void mdn::Mdn2dRules::internal_ncarryover(const Coord& xy) {
    Log_Debug3("");
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    #ifdef MDN_DEBUG
        if (abs(p) < m_config.dbase()) {
            std::ostringstream oss;
            oss << "Invalid carryover requested at " << xy << ": value (" << p << ") must exceed "
                << "base (" << m_config.dbase() << ")";
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
        Log_Debug4("Carryover with base " << static_cast<int>(m_config.base()) << " ("
            << static_cast<int>(p) << ":"
            << static_cast<int>(x) << ","
            << static_cast<int>(y) << "): result = ("
            << ip << ":" << ix << "," << iy << ")"
        );
    }
    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
}


void mdn::Mdn2dRules::internal_clearMetadata() const {
    Log_Debug3("");
    m_polymorphicNodes.clear();
    m_polymorphicNodes_event = -1;
    Mdn2dBase::internal_clearMetadata();
}
