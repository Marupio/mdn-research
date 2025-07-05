#include "Mdn2dRules.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include "Constants.h"
#include "Logger.h"
#include "MdnException.h"
#include "Tools.h"


mdn::Carryover mdn::Mdn2dRules::static_checkCarryover(Digit p, Digit x, Digit y, Digit base) {
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


mdn::Mdn2dRules::Mdn2dRules(int base, int precision)
:
    m_base(base),
    m_dbase(m_base),
    m_precision(precision),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_polymorphicNodes_event(-1),
    m_event(0)
{}


mdn::Mdn2dRules::Mdn2dRules(int base, int precision, int initVal)
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


mdn::Mdn2dRules::Mdn2dRules(int base, int precision, double initVal, Fraxis fraxis)
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


mdn::Mdn2dRules::Mdn2dRules(const Mdn2dRules& other):
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


mdn::Mdn2dRules& mdn::Mdn2dRules::operator=(const Mdn2dRules& other) {
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


mdn::Mdn2dRules::Mdn2dRules(Mdn2dRules&& other) noexcept :
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


mdn::Mdn2dRules& mdn::Mdn2dRules::operator=(Mdn2dRules&& other) noexcept {
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


mdn::Carryover mdn::Mdn2dRules::checkCarryover(const Coord& xy) const {
    auto lock = lockReadOnly();
    return locked_checkCarryover(xy);
}


mdn::Carryover mdn::Mdn2dRules::locked_checkCarryover(const Coord& xy) const {
    return static_checkCarryover(
        locked_getValue(xy),
        locked_getValue(xy.copyTranslateX(1)),
        locked_getValue(xy.copyTranslateY(1)),
        m_dbase
    );
}


void mdn::Mdn2dRules::carryover(const Coord& xy)
{
    auto lock = lockWriteable();
    modified();
    locked_carryover(xy);
}


std::vector<Coord> mdn::Mdn2dRules::locked_carryover(const Coord& xy, int carry) {
    Coord xy_x = xy.copyTranslateX(1);
    Coord xy_y = xy.copyTranslateY(1);
    Digit p = locked_getValue(xy);
    Digit x = locked_getValue(xy_x);
    Digit y = locked_getValue(xy_y);
    Carryover co = static_checkCarryover(p, x, y, m_dbase);
    if (co == Carryover::Invalid) {
        throw IllegalOperation("Invalid carryover requested at " + xy.to_string());
    }
    int ip = static_cast<int>(p) + carry;
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int nCarry = ip/m_base;
    #ifdef MDN_DEBUG
        if (nCarry > 1 || nCarry < -1) {
            std::ostringstream oss;
            oss << "Internal warning: carryover at " << xy << ": magnitude should not exceed 1. ";
            oss << "Got " << nCarry << "." << std::endl;
            Logger::instance().warn(oss.str());
        }
    #endif
    ip -= nCarry * m_base;
    iy += nCarry;
    ix += nCarry;

    std::vector<Coord> affectedCoords;
    if (iy > m_config.base() || iy < -(m_config.base())) {
        std::vector<Coord> ret = locked_carryover(xy_y, nCarry);
        affectedCoords.insert(affectedCoords.end(), ret.begin(), ret.end());
    }
    // TODO!!
    locked_setValue(xy, ip);
    locked_setValue(xy_x, ix);
    locked_setValue(xy_y, iy);
    return std::vector<Coord>({xy, xy_x, xy_y});
}


void mdn::Mdn2dRules::shift(int xDigits, int yDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shift(xDigits, yDigits);
}


void mdn::Mdn2dRules::shift(const Coord& xy) {
    auto lock = lockWriteable();
    modified();
    locked_shift(xy);
}


void mdn::Mdn2dRules::locked_shift(const Coord& xy) {
    locked_shift(xy.x(), xy.y());
}


void mdn::Mdn2dRules::locked_shift(int xDigits, int yDigits) {
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


void mdn::Mdn2dRules::shiftRight(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftRight(nDigits);
}


void mdn::Mdn2dRules::locked_shiftRight(int nDigits) {
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


void mdn::Mdn2dRules::shiftLeft(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftLeft(nDigits);
}


void mdn::Mdn2dRules::locked_shiftLeft(int nDigits) {
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


void mdn::Mdn2dRules::shiftUp(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftUp(nDigits);
}


void mdn::Mdn2dRules::locked_shiftUp(int nDigits) {
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


void mdn::Mdn2dRules::shiftDown(int nDigits) {
    auto lock = lockWriteable();
    modified();
    locked_shiftDown(nDigits);
}


void mdn::Mdn2dRules::locked_shiftDown(int nDigits) {
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


void mdn::Mdn2dRules::transpose() {
    auto lock = lockWriteable();
    modified();
    locked_transpose();
}


void mdn::Mdn2dRules::locked_transpose() {
    Mdn2dRules temp(m_base, m_precision);
    auto tempLock = temp.lockWriteable();
    for (const auto& [xy, digit] : m_raw) {
        temp.locked_setValue(Coord(xy.y(), xy.x()), digit);
    }
    operator=(temp);
}


const std::unordered_set<mdn::Coord>& mdn::Mdn2dRules::getPolymorphicNodes() const {
    auto lock = ReadOnlyLock();
    return locked_getPolymorphicNodes();
}


const std::unordered_set<mdn::Coord>& mdn::Mdn2dRules::locked_getPolymorphicNodes() const {
    if (m_polymorphicNodes_event != m_event) {
        internal_polymorphicScan();
    }
    return m_polymorphicNodes;
}


void mdn::Mdn2dRules::polymorphism_x0() {
    auto lock = lockWriteable();
    locked_polymorphism_x0();
}


void mdn::Mdn2dRules::locked_polymorphism_x0() {
    const std::unordered_set<Coord>& pn = locked_getPolymorphicNodes();
    for (const Coord& xy : pn) {
        Digit p = locked_getValue(xy);
        if (p > 0) {
            internal_oneCarryover(xy);
        }
    }
}


void mdn::Mdn2dRules::polymorphism_y0() {
    auto lock = lockWriteable();
    locked_polymorphism_y0();
}


void mdn::Mdn2dRules::locked_polymorphism_y0() {
    const std::unordered_set<Coord>& pn = locked_getPolymorphicNodes();
    for (const Coord& xy : pn) {
        Digit p = locked_getValue(xy);
        if (p < 0) {
            internal_oneCarryover(xy);
        }
    }
}


bool mdn::Mdn2dRules::operator==(const Mdn2dRules& rhs) const {
    auto lock = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    Mdn2dRules lhsCopy(*this);
    Mdn2dRules rhsCopy(rhs);
    lhsCopy.locked_polymorphism_x0();
    rhsCopy.locked_polymorphism_x0();
    return lhsCopy.m_raw == rhsCopy.m_raw;
}


bool mdn::Mdn2dRules::operator!=(const Mdn2dRules& rhs) const {
    return !(*this == rhs);
}


void mdn::Mdn2dRules::internal_updatePolymorphism() {
    m_polymorphicNodes.clear();
    m_polymorphicNodes_event = -1;
    for (const auto& [seedXy, digit] : m_raw) {
        switch(locked_checkCarryover(seedXy) {
            case Carryover::Required:
                locked_carryover(seedXy);
                required.push_back(seedXy);
                break;
            case Carryover::Optional:
                m_polymorphicNodes.insert(seedXy);
                break;
            default:
                break;
        }
    }
}


void mdn::Mdn2dRules::internal_polymorphicScanAndFix() {
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


void mdn::Mdn2dRules::internal_polymorphicScan() const {
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


void mdn::Mdn2dRules::internal_makePolymorphicTree() {
    m_polymorphicTree.clear();
    for (const auto& [xy, digit] : m_raw) {
        switch(locked_checkCarryover(xy)) {
            case Carryover::Required:
                throw InvalidState("Found required carryover at " +  xy.to_string());
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


void mdn::Mdn2dRules::internal_oneCarryover(const Coord& xy) {
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


void mdn::Mdn2dRules::internal_ncarryover(const Coord& xy) {
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


void mdn::Mdn2dRules::internal_clearMetadata() const override {
    m_polymorphicNodes.clear();
    m_polymorphicNodes_event = -1;
    Mdn2dBase::internal_clearMetadata();
}
