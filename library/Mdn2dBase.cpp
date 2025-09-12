#include "Mdn2dBase.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <filesystem>

#include "Constants.hpp"
#include "GlobalConfig.hpp"
#include "Logger.hpp"
#include "Mdn2d.hpp"
#include "Mdn2dIO.hpp"
#include "MdnException.hpp"
#include "Tools.hpp"


std::shared_mutex mdn::Mdn2dBase::m_static_mutex;

int mdn::Mdn2dBase::m_nextNameSeed = 0;


std::string mdn::Mdn2dBase::static_generateNextName(const std::string& prefix) {
    auto lock = std::unique_lock<std::shared_mutex>(m_static_mutex);
    Log_Debug3_H("")
    std::string newName = locked_generateNextName(prefix);
    Log_Debug3_T("returning name=" << newName);
    return newName;
}


std::string mdn::Mdn2dBase::locked_generateNextName(const std::string& prefix) {
    Log_Debug4_H("prefix=[" << prefix << "]");
    Mdn2dFramework& framework = Mdn2dConfig::master();
    if (framework.className() != "Mdn2dFramework") {
        std::string fs = framework.suggestName(prefix);
        Log_Debug4_T("framework suggests [" << fs << "]");
        return fs;
    }

    // No framework to go on
    std::string working = prefix.empty() ? "Mdn" : prefix;

    std::pair<std::string, int> prefixAndInt(Tools::strInt(working));
    std::string pf = prefixAndInt.first;
    int val = prefixAndInt.second;
    if (val < 0) {
        std::string newName = pf + std::to_string(++m_nextNameSeed);
        Log_Debug4_T("returning [" << newName << "]");
        return newName;
    }
    std::string newName = pf + std::to_string(val + 1);
    Log_Debug4_T("returning [" << newName << "]");
    return newName;
}


std::string mdn::Mdn2dBase::static_generateCopyName(const std::string& nameIn) {
    auto lock = std::unique_lock<std::shared_mutex>(m_static_mutex);
    Log_Debug3_H("")
    std::string newName = locked_generateCopyName(nameIn);
    Log_Debug3_T("Returning newName=" << newName);
    return newName;
}


std::string mdn::Mdn2dBase::locked_generateCopyName(const std::string& nameIn) {
    std::regex suffixRegex(R"(^(.*_Copy)(\d+)$)");
    std::smatch match;

    std::string candidate;
    Log_Debug4_H("");
    if (std::regex_match(nameIn, match, suffixRegex)) {
        Log_Debug4("Has suffix '_Copy#'");
        std::string base = match[1];
        int nCopy = std::stoi(match[2]) + 1;
        do {
            Log_Debug4("checking " << nCopy);
            candidate = base + std::to_string(nCopy++);
        } while (Mdn2dConfig::master().mdnNameExists(candidate));
        Log_Debug4_T("returning " << candidate);
        return candidate;
    }

    candidate = nameIn + "_Copy_0";
    Log_Debug4("No '_Copy_#' suffix, checking ..._Copy_0 availability");
    while (Mdn2dConfig::master().mdnNameExists(candidate)) {
        Log_Debug4("'__Copy_0' not available");
        static std::regex baseRegex(R"((.*_Copy_))");
        std::smatch baseMatch;
        if (std::regex_match(candidate, baseMatch, baseRegex)) {
            int n = 1;
            do {
                Log_Debug4("Checking " << n);
                candidate = baseMatch[1].str() + std::to_string(n++);
            } while (Mdn2dConfig::master().mdnNameExists(candidate));
        } else {
            break;
        }
    }
    Log_Debug4_T("Returning " << candidate);
    return candidate;
}


mdn::Mdn2d mdn::Mdn2dBase::NewInstance(Mdn2dConfig config, std::string nameIn) {
    Log_Debug_H("Preparing NewInstance with config=" << config << ", name=" << nameIn);
    std::string newName = nameIn;
    if (nameIn.empty()) {
        Log_Debug3("nameIn empty, generating new name");
        newName = static_generateNextName();
    }
    Log_Debug_T("Mdn2d constructor dispatch, newName=" << newName);
    return Mdn2d(config, newName);
}


mdn::Mdn2d mdn::Mdn2dBase::Duplicate(const Mdn2d& other, std::string nameIn) {
    Log_Debug2_H("Duplicating " << other.m_name << ", name=" << nameIn)

    std::string newName = nameIn;
    if (nameIn.empty()) {
        Log_Debug3("nameIn empty, generating new name from " << other.m_name);
        newName = static_generateCopyName(other.m_name);
    }
    If_Log_Showing_Debug2(
        Log_Debug2_T("Name of new Mdn2d will be " << newName);
    );
    If_Not_Log_Showing_Debug2(
        Log_Debug("Duplicating Mdn2d from " << other.m_name << ", name=" << newName);
    );
    return Mdn2d(other, newName);
}


mdn::Mdn2dBase::Mdn2dBase(std::string nameIn)
:
    m_config(Mdn2dConfig::static_defaultConfig()),
    m_name(nameIn),
    m_bounds(Rect::GetInvalid()),
    m_modified(false),
    m_event(0)
{
    Log_Debug3_H("null ctor, nameIn=" << m_name);
    if (m_name.empty()) {
        Log_Debug4("nameIn empty, generating new name");
        m_name = static_generateNextName();
        Log_Debug3("changed name to " << m_name);
    }
    Log_Debug3_T("");
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dConfig config, std::string nameIn)
:
    m_config(config),
    m_name(nameIn),
    m_bounds(Rect::GetInvalid()),
    m_modified(false),
    m_event(0)
{
    Log_Debug3_H("compenent ctor, config=" << config << ", nameIn=" << m_name);
    if (m_name.empty()) {
        Log_Debug4("nameIn empty, generating new name");
        m_name = static_generateNextName();
        Log_Debug3("changed name to " << m_name);
    }
    Log_Debug3_T("");
}


mdn::Mdn2dBase::Mdn2dBase(const Mdn2dBase& other, std::string nameIn):
    m_config(other.m_config),
    m_name(nameIn),
    m_modified(false),
    m_event(0)
{
    Log_Debug3_H("copy ctor, copying " << other.m_name << ", newName=" << nameIn);
    auto lock = other.lockReadOnly();
    if (m_name.empty()) {
        Log_Debug4("nameIn empty, generating new name from " << other.m_name);
        m_name = static_generateCopyName(other.m_name);
        Log_Debug3("changed name to " << m_name);
    }
    m_raw = other.m_raw;
    m_xIndex = other.m_xIndex;
    m_yIndex = other.m_yIndex;
    m_index = other.m_index;
    m_index = other.m_index;
    m_bounds = other.m_bounds;
    Log_Debug3_T("");
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(const Mdn2dBase& other) {
    Log_Debug3_H("Setting " << m_name << " equal to " << other.m_name);
    if (this != &other) {
        Log_Info("Getting writeable lock for this...");
        auto lockThis = lockWriteable();
        Log_Info("Getting readable lock for other...");
        auto lockOther = other.lockReadOnly();
        Log_Debug3("other is not me, operator= should work...");
        if (m_config.base() != other.m_config.base()) {
            BaseMismatch err(m_config.base(), other.m_config.base());
            Log_Error(err.what());
            throw err;
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = other.m_raw;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_index = other.m_index;
        m_bounds = other.m_bounds;
        internal_modified();
    } else {
        Log_Warn("Attempting to set Mdn2d equal to itself");
    }
    Log_Debug3_T("");
    return *this;
}


mdn::Mdn2dBase& mdn::Mdn2dBase::locked_operatorEquals(const Mdn2dBase& other) {
    Log_Debug3_H("Setting " << m_name << " equal to " << other.m_name);
    if (this != &other) {
        Log_Debug3("other is not me, operator= should work...");
        if (m_config.base() != other.m_config.base()) {
            BaseMismatch err(m_config.base(), other.m_config.base());
            Log_Error(err.what());
            throw err;
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = other.m_raw;
        m_xIndex = other.m_xIndex;
        m_yIndex = other.m_yIndex;
        m_index = other.m_index;
        m_bounds = other.m_bounds;
        internal_modified();
    } else {
        Log_Warn("Attempting to set Mdn2d equal to itself");
    }
    Log_Debug3_T("");
    return *this;
}


mdn::Mdn2dBase::Mdn2dBase(Mdn2dBase&& other) noexcept :
    m_config(other.m_config),
    m_name(other.m_name),
    m_modified(false),
    m_event(0)
{
    Log_Debug3_H("move-copy ctor, copying " << other.m_name);
    auto lock = other.lockReadOnly();

    m_raw = std::move(other.m_raw);
    m_xIndex = std::move(other.m_xIndex);
    m_yIndex = std::move(other.m_yIndex);
    m_index = std::move(other.m_index);
    m_bounds = other.m_bounds;
    Log_Debug3_T("");
}


mdn::Mdn2dBase& mdn::Mdn2dBase::operator=(Mdn2dBase&& other) noexcept {
    Log_Debug3_H("Setting " << m_name << " move-equal to " << other.m_name);
    if (this != &other) {
        auto lockThis = lockWriteable();
        auto lockOther = other.lockReadOnly();
        Log_Debug3("other is not me, operator= should work...");
        if (m_config.base() != other.m_config.base()) {
            BaseMismatch err(m_config.base(), other.m_config.base());
            Log_Error(err.what());
            //throw err;
            m_config.setInvalid(err.what());
        }
        m_config.setPrecision(other.m_config.precision());
        m_raw = std::move(other.m_raw);
        m_xIndex = std::move(other.m_xIndex);
        m_yIndex = std::move(other.m_yIndex);
        m_index = std::move(other.m_index);
        m_bounds = other.m_bounds;
        internal_modified();
    } else {
        Log_Warn("Attempting to set Mdn2d equal to itself");
    }
    Log_Debug3_T("");
    return *this;
}


mdn::Mdn2dBase::~Mdn2dBase() {
    Log_N_Debug4_H("");
    for (auto& [id, obs] : m_observers) {
        Log_N_Debug4("farewell call on " << id);
        obs->farewell();
    }
    Log_N_Debug4_T("");
}


const mdn::Mdn2dConfig& mdn::Mdn2dBase::config() const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    return locked_config();
}


const mdn::Mdn2dConfig& mdn::Mdn2dBase::locked_config() const {
    Log_N_Debug4("returning config: " << m_config);
    return m_config;
}


mdn::Mdn2dConfigImpact mdn::Mdn2dBase::assessConfigChange(const Mdn2dConfig& newConfig) const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    return locked_assessConfigChange(newConfig);
}


mdn::Mdn2dConfigImpact mdn::Mdn2dBase::locked_assessConfigChange(
    const Mdn2dConfig& newConfig
) const {
    Log_N_Debug3_H("assessing new: " << newConfig << " against old: " << m_config);

    Mdn2dConfigImpact result = Mdn2dConfigImpact::Unknown;
    if (newConfig.base() != m_config.base()) {
        result = Mdn2dConfigImpact::AllDigitsCleared;
    } else if (newConfig.precision() < m_config.precision()) {
        if (newConfig.signConvention() != m_config.signConvention()) {
            result = Mdn2dConfigImpact::PossibleDigitLossAndPolymorphism;
        }
        result = Mdn2dConfigImpact::PossibleDigitLoss;
    } else if (newConfig.signConvention() != m_config.signConvention()) {
        result = Mdn2dConfigImpact::PossiblePolymorphism;
    } else {
        result = Mdn2dConfigImpact::NoImpact;
    }
    Log_N_Debug3_T(
        "result = " << Mdn2dConfigImpactToName(result) << ", "
            << Mdn2dConfigImpactToDescription(result)
    );
    return result;
}


void mdn::Mdn2dBase::setConfig(const Mdn2dConfig& newConfig) {
    Log_N_Debug2("");
    auto lock = lockWriteable();
    locked_setConfig(newConfig);
}


void mdn::Mdn2dBase::locked_setConfig(const Mdn2dConfig& newConfig) {
    Log_N_Debug3_H("applying new config: " << newConfig);

    if (newConfig.base() != m_config.base()) {
        Log_N_Debug4("Requires full clear()");
        locked_clear();
    } else if (newConfig.precision() < m_config.precision()) {
        if (newConfig.signConvention() != m_config.signConvention()) {
            // Can only be handled by Rules layer
            // result = Mdn2dConfigImpact::PossibleDigitLossAndPolymorphism;
        }
        Log_N_Debug4("Setting reduced precision - may lose digits");
        int nLost = locked_setPrecision(newConfig.precision());
        Log_N_Debug3("Precision reduction lost " << nLost << " digits");
    } else if (newConfig.signConvention() != m_config.signConvention()) {
        // Can only be handled by Rules layer
        // result = Mdn2dConfigImpact::PossiblePolymorphism;
    }
    m_config = newConfig;
    Log_N_Debug3_T("");
}


void mdn::Mdn2dBase::registerObserver(MdnObserver* obs) const {
    Log_N_Debug2("");
    auto lock = lockWriteable();
    locked_registerObserver(obs);
}


void mdn::Mdn2dBase::locked_registerObserver(MdnObserver* obs) const {
    static int instance = 0;
    Log_N_Debug3("Registering " << instance);
    obs->setInstance(instance);
    m_observers.insert({instance++, obs});
}


void mdn::Mdn2dBase::unregisterObserver(MdnObserver* obs) const {
    Log_N_Debug2("");
    auto lock = lockWriteable();
    locked_unregisterObserver(obs);
}


void mdn::Mdn2dBase::locked_unregisterObserver(MdnObserver* obs) const {
    Log_N_Debug3_H("");
    if (!obs) {
        Log_Warn("Attempting to unregister invalid observer");
        Log_N_Debug3_T("");
        return;
    }
    auto iter = m_observers.find(obs->instance());
    if (iter == m_observers.end()) {
        Log_Warn(
            "Attempting to unregister observer " << obs->instance() << ", attached to Mdn2d '"
            << obs->get()->name() << "' from incorrect Mdn2d '" << locked_name() << "'"
        );
        Log_N_Debug3_T("");
        return;
    }
    m_observers.erase(obs->instance());
    Log_N_Debug3_T("");
}


const std::string& mdn::Mdn2dBase::name() const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    return locked_name();
}


const std::string& mdn::Mdn2dBase::locked_name() const {
    Log_Debug3("returning name=" << m_name);
    return m_name;
}


std::string mdn::Mdn2dBase::setName(const std::string& nameIn) {
    Log_N_Debug2("nameIn=" << nameIn);
    auto lock = lockWriteable();
    return locked_setName(nameIn);
}


std::string mdn::Mdn2dBase::locked_setName(const std::string& nameIn) {
    std::string fwName = m_config.master().requestMdnNameChange(m_name, nameIn);
    Log_Debug3(
        "Attempting to change name from '" << m_name << "' to '" << nameIn << "', with framework"
        << " final approval as '" << fwName << "'"
    );
    m_name = fwName;
    return fwName;
}


bool mdn::Mdn2dBase::nonZero(const Coord& xy) const {
    Log_N_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    bool result = locked_nonZero(xy);
    Log_N_Debug2_T("result=" << result);
    return result;
}
bool mdn::Mdn2dBase::locked_nonZero(const Coord& xy) const {
    Log_Debug4("");
    return m_index.find(xy) != m_index.end();
}


const mdn::CoordSet& mdn::Mdn2dBase::nonZeroOnRow(const Coord& xy) const {
    Log_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    const CoordSet& result = locked_nonZeroOnRow(xy);
    Log_Debug2_T("Returning " << result.size() << " non-zero values");
    return result;
}
const mdn::CoordSet& mdn::Mdn2dBase::locked_nonZeroOnRow(const Coord& xy) const {
    Log_Debug3_H("At " << xy);
    auto it = m_yIndex.find(xy.y());
    if (it != m_yIndex.end()) {
        const CoordSet& coords = it->second;
        If_Log_Showing_Debug4(
            std::string coordsList(Tools::setToString<Coord>(coords, ','));
            Log_N_Debug4_T("Returning non-zero coords: " << coordsList);
        );
        If_Not_Log_Showing_Debug4(
            Log_N_Debug3_T("Returning set containing " << coords.size() << " non-zero coords");
        );
        return coords;
    }
    Log_Debug3_T("Returning empty set");
    return m_nullCoordSet;
}


const mdn::CoordSet& mdn::Mdn2dBase::nonZeroOnCol(const Coord& xy) const {
    Log_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    const CoordSet& result = locked_nonZeroOnCol(xy);
    Log_Debug2_T("Returning " << result.size() << " non-zero values");
    return result;
}
const mdn::CoordSet& mdn::Mdn2dBase::locked_nonZeroOnCol(const Coord& xy) const {
    Log_Debug3_H("At " << xy);
    auto it = m_xIndex.find(xy.x());
    if (it != m_xIndex.end()) {
        const CoordSet& coords = it->second;
        If_Log_Showing_Debug4(
            std::string coordsList(Tools::setToString<Coord>(coords, ','));
            Log_N_Debug4_T("Returning non-zero coords: " << coordsList);
        );
        If_Not_Log_Showing_Debug4(
            Log_N_Debug3_T("Returning set containing " << coords.size() << " non-zero coords");
        );
        return coords;
    }
    Log_Debug3_T("Returning empty set");
    return m_nullCoordSet;
}


mdn::Coord mdn::Mdn2dBase::jump(const Coord& xy, CardinalDirection cd) const {
    Log_N_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    Coord result = locked_jump(xy, cd);
    Log_N_Debug2_T("result=" << result);
    return result;
}


mdn::Coord mdn::Mdn2dBase::locked_jump(const Coord& xy, CardinalDirection cd) const {
    const Coord& cdCoord(CardinalDirectionToCoord(cd));
    Log_N_Debug3_H("xy=" << xy << ", cd=" << cdCoord);
    if (cdCoord ==  COORD_ORIGIN) {
        Log_N_Debug3_T("Not a valid cardinal direction");
    }
    // When true, movement towards digit line is only allowed
    bool towardsDigitLineOnly = false;
    Rect::FrontBack fbX;
    Rect::FrontBack fbY;
    if (m_bounds.isInvalid()) {
        Log_N_Debug3("No digits, only allowed movement towards digit lines");
        towardsDigitLineOnly = true;
    } else if (cdCoord.x() != 0) {
        fbX = m_bounds.HasCoordAt_X(xy);
        fbY = m_bounds.HasCoordAt_Y(xy);

        switch(fbY) {
            case Rect::FrontBack::Behind:
            case Rect::FrontBack::InFront:
                // Above or below bounds, moving sideways
                towardsDigitLineOnly = true;
                break;
            default:
                break;
        }
        switch (fbX) {
            case Rect::FrontBack::Behind:
            case Rect::FrontBack::BackEdge: {
                if (cdCoord.x() < 0) {
                    Log_Debug3_T("Outside bounds moving away, no movement allowed");
                    return Coord(xy);
                }
                // Heading towards bounds
                break;
            }
            case Rect::FrontBack::InFront:
            case Rect::FrontBack::FrontEdge: {
                if (cdCoord.x() > 0) {
                    Log_Debug3_T("Outside bounds moving away, no movement allowed");
                    return Coord(xy);
                }
                // Heading towards bounds
                break;
            }
            default:
                break;
        }
    } else if (cdCoord.y() != 0) {
        fbX = m_bounds.HasCoordAt_X(xy);
        fbY = m_bounds.HasCoordAt_Y(xy);

        switch(fbX) {
            case Rect::FrontBack::Behind:
            case Rect::FrontBack::InFront:
                // To the left or right, moving vertically
                towardsDigitLineOnly = true;
                break;
            default:
                break;
        }
        switch (fbY) {
            case Rect::FrontBack::Behind:
            case Rect::FrontBack::BackEdge: {
                if (cdCoord.y() < 0) {
                    Log_Debug3_T("Outside bounds moving away, no movement allowed");
                    return Coord(xy);
                }
                // Heading towards bounds
                break;
            }
            case Rect::FrontBack::InFront:
            case Rect::FrontBack::FrontEdge: {
                if (cdCoord.y() > 0) {
                    Log_Debug3_T("Outside bounds moving away, no movement allowed");
                    return Coord(xy);
                }
                // Heading towards bounds
                break;
            }
            default:
                break;
        }
    }
    if (towardsDigitLineOnly) {
        // Only allow towards digit line
        // Multiply components, then 'toward digit line' is a negative coord.
        Log_N_Debug3("No digits, only allowed movement towards digit lines");

        Coord xy_cd = xy*cdCoord.x();
        if (xy_cd.x() < 0) {
            Coord result(0, xy.y());
            Log_N_Debug3_T("Towards x digit line, returning " << result);
            return result;
        } else if (xy_cd.y() < 0) {
            Coord result(xy.x(), 0);
            Log_N_Debug3_T("Towards y digit line, returning " << result);
            return result;
        }
        // Not moving towards a digitLine, no movement allowed
        Log_N_Debug3_T(
            "Only allowed digitLine-ward movement, not correct movement input, returning " << xy
        );
        return xy;
    }

    // Begin standard jump algorithm - move until non-zero status changes
    bool refNonZero = locked_nonZero(xy);
    const int sanityCheckMax = 1000;
    int sanityCheck = sanityCheckMax;
    // Coord xyGo(xy.translated(cdCoord));
    Coord xyGo = xy;
    Coord prevXy = xy;

    while (sanityCheck--> 0) {
        prevXy = xyGo;
        xyGo.translate(cdCoord);
        if (locked_nonZero(xyGo) != refNonZero) {
            if (refNonZero) {
                // We want to return non-zero location, so, previous spot is okay
                Log_N_Debug3_T("returning " << prevXy);
                return prevXy;
            } else {
                // We want to return non-zero location, so, current spot is okay
                Log_N_Debug3_T("returning " << xyGo);
                return xyGo;
            }
        }
        if (!m_bounds.contains(xyGo)) {
            break;
        }
    }

    if (sanityCheck < 0) {
        Log_N_Warn("jump algorithm exceeded " << sanityCheckMax << " iterations");
    }
    // No valid stops - went out-of-bounds. Return the previous position.
    Log_N_Debug3_T("no valid stops found, went out-of-bounds, returning " << prevXy);
    return prevXy;
}


mdn::Digit mdn::Mdn2dBase::getValue(const Coord& xy) const {
    Log_N_Debug2_H("At " << xy);
    auto lock = lockReadOnly();
    Digit result = locked_getValue(xy);
    Log_N_Debug2_T("result=" << static_cast<int>(result));
    return result;
}


mdn::Digit mdn::Mdn2dBase::locked_getValue(const Coord& xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        Log_N_Debug3("At " << xy << ": no entry, returning 0");
        return static_cast<Digit>(0);
    }
    If_Log_Showing_Debug3(
        Log_N_Debug3("At " << xy << ": returning " << static_cast<int>(it->second));
    );
    return it->second;
}


mdn::VecDigit mdn::Mdn2dBase::getRow(int y) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    VecDigit result = locked_getRow(y);
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Row " << y << ": " << Tools::digitArrayToString(result));
    );
    return result;
}


mdn::VecDigit mdn::Mdn2dBase::locked_getRow(int y) const {
    VecDigit digits;
    Log_N_Debug3_H("Row " << y);
    locked_getRow(y, digits);
    Log_N_Debug3_T("result=[array with " << digits.size() << " elements]");
    return digits;
}


void mdn::Mdn2dBase::getRow(int y, VecDigit& out) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    locked_getRow(y, out);
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Row " << y << ": inplace" << Tools::digitArrayToString(out));
    );
}


void mdn::Mdn2dBase::locked_getRow(int y, VecDigit& out) const {
    Log_N_Debug3_H("y=" << y);
    if (!m_bounds.isValid()) {
        // No non-zero digits to fill
        if (out.size()) {
            std::fill(out.begin(), out.end(), Digit(0));
        }
        Log_N_Debug3_T("no bounds");
        return;
    }
    locked_getRow(Coord(m_bounds.min().x(), y), m_bounds.width(), out);
    Log_N_Debug3_T("");
}


void mdn::Mdn2dBase::getRow(const Coord& xy, VecDigit& out) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    locked_getRow(xy, out);
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Row " << xy << ": inplace" << Tools::digitArrayToString(out));
    );
}


void mdn::Mdn2dBase::locked_getRow(const Coord& xy, VecDigit& out) const {
    Log_N_Debug3_H("xy=" << xy);
    if (!m_bounds.isValid()) {
        // No non-zero digits to fill
        if (out.size()) {
            std::fill(out.begin(), out.end(), Digit(0));
        }
        Log_N_Debug3_T("no bounds");
        return;
    }
    // int xStart = m_bounds.min().x();
    // int xEnd = m_bounds.max().x() + 1;
    locked_getRow(xy, m_bounds.width(), out);
    Log_N_Debug3_T("");
}


// void mdn::Mdn2dBase::getRow(int y, int x0, int x1, VecDigit& out) const {
//     auto lock = lockReadOnly();
//     Log_N_Debug2("");
//     locked_getRow(y, x0, x1, out);
// }


// void mdn::Mdn2dBase::locked_getRow(int y, int x0, int x1, VecDigit& out) const
// {
//     int xCount = x1 - x0 + 1;
//     Log_N_Debug3_H(
//         "Row " << y << " from x (" << x0 << " .. " << x1 << "), "
//         << xCount << " elements"
//     );
//     out.resize(xCount);
//     std::fill(out.begin(), out.end(), 0);
//     auto it = m_yIndex.find(y);
//     if (it != m_yIndex.end()) {
//         // There are non-zero entries on this row, fill them in
//         const CoordSet& coords = it->second;
//         for (const Coord& coord : coords) {
//             if (coord.x() >= x0 && coord.x() <= x1) {
//                 out[coord.x()-x0] = m_raw.at(coord);
//             }
//         }
//     }
//     Log_N_Debug3_T("");
// }


void mdn::Mdn2dBase::getRow(const Coord& xy, int width, VecDigit& out) const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    locked_getRow(xy, width, out);
}


void mdn::Mdn2dBase::locked_getRow(const Coord& xy, int width, VecDigit& out) const
{
    int x0 = xy.x();
    int y = xy.y();
    int x1 = x0 + width - 1;
    Log_N_Debug3_H(
        "Row " << y << " from x (" << x0 << " .. " << x1 << "), "
        << width << " elements"
    );
    out.resize(width);
    std::fill(out.begin(), out.end(), 0);
    auto it = m_yIndex.find(y);
    if (it != m_yIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            if (coord.x() >= x0 && coord.x() <= x1) {
                out[coord.x()-x0] = m_raw.at(coord);
            }
        }
    }
    Log_N_Debug3_T("");
}


mdn::Rect mdn::Mdn2dBase::getAreaRows(VecVecDigit& out) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    locked_getAreaRows(m_bounds, out);
    Log_N_Debug2_T("");
    return m_bounds;
}


void mdn::Mdn2dBase::getAreaRows(const Rect& window, VecVecDigit& out) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    locked_getAreaRows(window, out);
    Log_N_Debug2_T("");
}


void mdn::Mdn2dBase::locked_getAreaRows(const Rect& window, VecVecDigit& out) const {
    int xStart = window.min().x();
    int xEnd = window.max().x()+1;
    int width = xEnd - xStart;

    int yStart = window.min().y();
    int yEnd = window.max().y()+1;
    int yCount = yEnd - yStart;

    Log_N_Debug3_H(
        "Converting sparse storage to digit arrays covering window with:\n"
        << "    xRange = (" << xStart << " .. " << xEnd << "), " << width << " elements,\n"
        << "    yRange = (" << yStart << " .. " << yEnd << "), " << yCount << " rows"
    );

    out.clear();
    out.reserve(yCount);

    for (int y = yStart; y < yEnd; ++y) {
        VecDigit row(width, Digit(0));
        Coord xy(xStart, y);
        locked_getRow(xy, width, row);
        out.push_back(row);
    }
    Log_N_Debug3_T("");
}


mdn::VecDigit mdn::Mdn2dBase::getCol(int x) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    VecDigit result = locked_getCol(x);
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Column " << x << ": " << Tools::digitArrayToString(result));
    );
    return result;
}


mdn::VecDigit mdn::Mdn2dBase::locked_getCol(int x) const {
    VecDigit digits;
    Log_N_Debug3_H("Column " << x);
    locked_getCol(x, digits);
    Log_N_Debug3_T("result=[array with " << digits.size() << " elements]");
    return digits;
}


void mdn::Mdn2dBase::getCol(int x, VecDigit& out) const {
    Log_N_Debug2_H("Column " << x);
    auto lock = lockReadOnly();
    locked_getCol(x, out);
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Column " << x << ": inplace" << Tools::digitArrayToString(out));
    );
}


void mdn::Mdn2dBase::locked_getCol(int x, VecDigit& out) const {
    Log_N_Debug3_H("x=" << x);
    if (!m_bounds.isValid()) {
        // No non-zero digits to fill
        if (out.size()) {
            std::fill(out.begin(), out.end(), Digit(0));
        }
        Log_N_Debug3_T("no bounds");
        return;
    }
    Coord xy(x, m_bounds.min().y());
    locked_getCol(xy, m_bounds.height(), out);
    Log_N_Debug3_T("");
}


void mdn::Mdn2dBase::getCol(const Coord& xy, int height, VecDigit& out) const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    locked_getCol(xy, height, out);
}


void mdn::Mdn2dBase::locked_getCol(const Coord& xy, int height, VecDigit& out) const
{
    int y0 = xy.y();
    int x = xy.x();
    int y1 = height + y0 - 1;
    Log_N_Debug3_H(
        "Col " << x << " from y (" << y0 << " .. " << y1 << "), "
        << height << " elements"
    );
    out.resize(height);
    std::fill(out.begin(), out.end(), 0);
    auto it = m_xIndex.find(x);
    if (it != m_xIndex.end()) {
        // There are non-zero entries on this row, fill them in
        const CoordSet& coords = it->second;
        for (const Coord& coord : coords) {
            if (coord.y() >= y0 && coord.y() <= y1) {
                out[coord.y()-y0] = m_raw.at(coord);
            }
        }
    }
    Log_N_Debug3_T("");
}


mdn::CoordSet mdn::Mdn2dBase::getNonZeroes(const Rect& window) const {
    Log_N_Debug2_H("Collecting non-zero coords in window: " << window);
    auto lock = lockReadOnly();
    CoordSet result = locked_getNonZeroes(window);
    Log_N_Debug2_T("Returning set containing " << result.size() << " coords");
    return result;
}


mdn::CoordSet mdn::Mdn2dBase::locked_getNonZeroes(const Rect& window) const {
    CoordSet result;

    // Fast-outs
    if (window.isInvalid()) {
        Log_N_Debug3("Window invalid; returning empty set");
        return result;
    }
    if (!locked_hasBounds()) {
        Log_N_Debug3("No bounds; MDN has no non-zero digits");
        return result;
    }

    // Clamp query to our current non-zero bounds
    Rect w = Rect::Intersection(window, m_bounds);
    if (!w.isValid()) {
        Log_N_Debug3("Intersection is empty; returning empty set");
        return result;
    }

    // Iterate by Y rows using m_yIndex; filter X on each row
    const int y0 = w.bottom();
    const int y1 = w.top();
    const int x0 = w.left();
    const int x1 = w.right();

    // lower_bound..upper_bound over y
    auto it = m_yIndex.lower_bound(y0);
    const auto itEnd = m_yIndex.upper_bound(y1);

    If_Log_Showing_Debug3(
        Log_N_Debug3_H(
            "Row scan y=(" << y0 << ".." << y1
            << "), x filter=(" << x0 << ".." << x1 << ")"
        );
    );

    for (; it != itEnd; ++it) {
        const CoordSet& rowSet = it->second;
        for (const Coord& c : rowSet) {
            const int x = c.x();
            if (x >= x0 && x <= x1) {
                result.insert(c);
            }
        }
    }

    If_Log_Showing_Debug4(
        std::string coordsList(Tools::setToString<Coord>(result, ','));
        Log_N_Debug4_T("Found " << result.size() << " coords: " << coordsList);
    );
    If_Not_Log_Showing_Debug4(
        Log_N_Debug3_T("Found " << result.size() << " coords");
    );
    return result;
}


void mdn::Mdn2dBase::clear() {
    Log_N_Debug2_H("");
    auto lock = lockWriteable();
    locked_clear();
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2dBase::locked_clear() {
    Log_N_Debug3_H("");
    m_raw.clear();
    internal_clearMetadata();
    Log_N_Debug3_T("");
}


bool mdn::Mdn2dBase::setToZero(const Coord& xy) {
    Log_N_Debug2_H("Setting " << xy << " to zero");
    auto lock = lockWriteable();
    bool possibleCarryOver = locked_setToZero(xy);
    internal_operationComplete();
    Log_N_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::locked_setToZero(const Coord& xy) {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // Already zero
        Log_N_Debug3_H("Setting " << xy << " to zero: already zero");
        bool result = locked_checkPrecisionWindow(xy) != PrecisionStatus::Below;
        Log_N_Debug3_T("result=" << result);
        return result;
    }
    // There is currently a non-zero value - erase it
    If_Log_Showing_Debug3(
        Log_N_Debug3_H(
            "Setting " << xy << " to zero: current value=" << static_cast<int>(it->second)
        );
    );
    auto xit(m_xIndex.find(xy.x()));
    auto yit(m_yIndex.find(xy.y()));
    #ifdef MDN_DEBUG
        // Debug mode - sanity check - metadata entries must be non-zero
        if (xit == m_xIndex.end() || yit == m_yIndex.end()) {
            Log_N_Warn(
                "Internal error: addressing data invalid, discovered when zeroing "
                << "coord: " << xy << "\n"
                << "Rebuilding metadata.\n"
            );
            locked_rebuildMetadata();
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
        Log_N_Debug3("Erasing empty indexing column at " << xy.x());
        m_xIndex.erase(xit);
        checkBounds = true;
    }

    if (coordsAlongY.size() == 0) {
        Log_N_Debug3("Erasing empty indexing row at " << xy.y());
        m_yIndex.erase(yit);
        checkBounds = true;
    }
    if (checkBounds) {
        // Bounds may have changed
        Log_N_Debug3("Updating bounds");
        internal_updateBounds();
    }
    Log_N_Debug3_T("result=1");
    return true;
}


mdn::CoordSet mdn::Mdn2dBase::setToZero(const CoordSet& coords) {
    If_Log_Showing_Debug3(
        std::string coordsList(Tools::setToString<Coord>(coords, ','));
        Log_N_Debug3_H("Zeroing coord set: " << coordsList);
    );
    If_Not_Log_Showing_Debug3(
        Log_N_Debug2_H("Zeroing set containing " << coords.size() << " coords");
    );
    auto lock = lockWriteable();
    CoordSet changed = locked_setToZero(coords);
    internal_operationComplete();
    If_Log_Showing_Debug3(
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug3_T("Returning changed coords: " << coordsList);
    );
    If_Not_Log_Showing_Debug3(
        Log_N_Debug2_T("Returning set containing " << changed.size() << " changed coords");
    );
    return changed;
}


mdn::CoordSet mdn::Mdn2dBase::locked_setToZero(const CoordSet& purgeSet) {
    Log_N_Debug3_H("Zeroing set containing " << purgeSet.size() << " coords");

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
    Log_N_Debug3_T(
        "Erased " << indexRowsRemoved << " index rows and " << indexColsRemoved << " index cols"
    );

    return changed;
}


mdn::CoordSet mdn::Mdn2dBase::setToZero(const Rect& window) {
    Log_N_Debug2_H("Zeroing window: " << window);
    auto lock = lockWriteable();
    CoordSet changed = locked_setToZero(window);
    internal_operationComplete();
    If_Log_Showing_Debug3(
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug3_T("Returning changed coords: " << coordsList);
    );
    If_Not_Log_Showing_Debug3(
        Log_N_Debug2_T("Returning set containing " << changed.size() << " changed coords");
    );
    return changed;
}


mdn::CoordSet mdn::Mdn2dBase::locked_setToZero(const Rect& window) {
    Log_N_Debug3_H("window=" << window);
    if (window.isInvalid()) {
        Log_N_Debug3_T("window empty");
        return CoordSet();
    }
    CoordSet purgeSet(
        locked_getNonZeroes(window)
    );
    CoordSet changed(locked_setToZero(purgeSet));
    If_Log_Showing_Debug4(
        std::string coordsList(Tools::setToString<Coord>(changed, ','));
        Log_N_Debug4_T("Returning changed coords: " << coordsList);
    );
    If_Not_Log_Showing_Debug4(
        Log_N_Debug3_T("Returning set containing " << changed.size() << " changed coords");
    );
    return changed;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, Digit value) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("Setting " << xy << " to " << static_cast<int>(value));
    );
    auto lock = lockWriteable();
    bool possibleCarryOver = locked_setValue(xy, value);
    internal_modifiedAndComplete();
    Log_N_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, int value) {
    Log_N_Debug2_H("Setting " << xy << " to " << value);
    auto lock = lockWriteable();
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_N_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long value) {
    Log_N_Debug2_H("Setting " << xy << " to " << value);
    auto lock = lockWriteable();
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_N_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::setValue(const Coord& xy, long long value) {
    Log_N_Debug2_H("Setting " << xy << " to " << value);
    auto lock = lockWriteable();
    bool possibleCarryOver = locked_setValue(xy, static_cast<Digit>(value));
    internal_modifiedAndComplete();
    Log_N_Debug2_T("result=" << possibleCarryOver);
    return possibleCarryOver;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, Digit value) {
    Log_N_Debug3_H("Setting " << xy << " to " << static_cast<int>(value));
    internal_checkDigit(xy, value);
    bool result = internal_setValueRaw(xy, value);
    Log_N_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, int value) {
    Log_N_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_N_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, long value) {
    Log_N_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_N_Debug3_T("result=" << result);
    return result;
}


bool mdn::Mdn2dBase::locked_setValue(const Coord& xy, long long value) {
    Log_N_Debug3_H("Setting " << xy << " to " << value);
    internal_checkDigit(xy, value);
    Digit dval = static_cast<Digit>(value);
    bool result = internal_setValueRaw(xy, dval);
    Log_N_Debug3_T("result=" << result);
    return result;
}


void mdn::Mdn2dBase::setRow(const Coord& xy, const VecDigit& row) {
    Log_N_Debug2_H("at " << xy);
    auto lock = lockWriteable();
    locked_setRow(xy, row);
    Log_N_Debug2_T("");
}


void mdn::Mdn2dBase::locked_setRow(const Coord& xy, const VecDigit& row) {
    Log_N_Debug3_H("at " << xy);
    Coord cursor = xy;
    for (int i = 0; i < row.size(); ++i) {
        locked_setValue(cursor, row[i]);
        cursor.translateX(1);
    }
    Log_N_Debug3_T("");
}


std::vector<std::string> mdn::Mdn2dBase::toStringRows() const {
    return toStringRows(TextWriteOptions::DefaultPretty());
}


std::vector<std::string> mdn::Mdn2dBase::toStringRows(const TextWriteOptions& opt) const {
    return mdn::Mdn2dIO::toStringRows(*this, opt);
}


std::vector<std::string> mdn::Mdn2dBase::toStringCols() const {
    return toStringCols(TextWriteOptions::DefaultPretty());
}


std::vector<std::string> mdn::Mdn2dBase::toStringCols(const TextWriteOptions& opt) const {
    return mdn::Mdn2dIO::toStringCols(*this, opt);
}


std::vector<std::string> mdn::Mdn2dBase::saveTextPrettyRows(
    bool wideNegatives, bool alphanumeric
) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    std::vector<std::string> result =
        locked_saveTextPrettyRows(m_bounds, wideNegatives, alphanumeric);
    Log_N_Debug2_T("result = " << result.size() << " rows of text");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::saveTextPrettyRows(
    Rect& window, bool wideNegatives, bool alphanumeric
) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    std::vector<std::string> result =
        locked_saveTextPrettyRows(window, wideNegatives, alphanumeric);
    Log_N_Debug2_T("result = " << result.size() << " rows of text");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::locked_saveTextPrettyRows(
    Rect& window, bool wideNegatives, bool alphanumeric
) const {
    Log_N_Debug3_H("");
    TextWriteOptions opt(TextWriteOptions::DefaultPretty());
    opt.wideNegatives = wideNegatives;
    opt.alphanumeric = alphanumeric;
    std::vector<std::string> result(Mdn2dIO::locked_toStringRows(*this, opt));
    Log_N_Debug3_T("");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::saveTextUtilityRows(CommaTabSpace delim) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    std::vector<std::string> result =
        locked_saveTextUtilityRows(m_bounds, delim);
    Log_N_Debug2_T("result = " << result.size() << " rows of text");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::saveTextUtilityRows(
    Rect& window, CommaTabSpace delim
) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    std::vector<std::string> result =
        locked_saveTextUtilityRows(window, delim);
    Log_N_Debug2_T("result = " << result.size() << " rows of text");
    return result;
}


std::vector<std::string> mdn::Mdn2dBase::locked_saveTextUtilityRows(
    Rect& window, CommaTabSpace delim
) const {
    Log_N_Debug3_H("");
    TextWriteOptions opt(TextWriteOptions::DefaultUtility());
    opt.delim = delim;
    std::vector<std::string> result(Mdn2dIO::locked_toStringRows(*this, opt));
    Log_N_Debug3_T("");
    return result;
}


void mdn::Mdn2dBase::saveTextPretty(std::ostream& os,
    bool wideNegatives,
    bool alphanumeric
) const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    locked_saveTextPretty(os, wideNegatives, alphanumeric);
}


void mdn::Mdn2dBase::locked_saveTextPretty(
    std::ostream& os,
    bool wideNegatives,
    bool alphanumeric
) const {
    std::vector<std::string> txt = locked_saveTextPrettyRows(m_bounds, wideNegatives, alphanumeric);
    os << m_bounds << '\n';
    for (auto riter = txt.rbegin(); riter != txt.rend(); ++riter) {
        os << *riter << std::endl;
    }
}


void mdn::Mdn2dBase::saveTextUtility(std::ostream& os, CommaTabSpace delim) const {
    Log_N_Debug2("");
    auto lock = lockReadOnly();
    locked_saveTextUtility(os, delim);
}


void mdn::Mdn2dBase::locked_saveTextUtility(std::ostream& os, CommaTabSpace delim) const {
    std::vector<std::string> txt = locked_saveTextUtilityRows(m_bounds, delim);
    os << m_bounds << '\n';
    for (auto riter = txt.rbegin(); riter != txt.rend(); ++riter) {
        os << *riter << std::endl;
    }
}


void mdn::Mdn2dBase::loadText(std::istream& is) {
    Log_N_Debug_H("");
    auto lock = lockWriteable();
    locked_loadText(is);
    Log_N_Debug_T("");
}


void mdn::Mdn2dBase::locked_loadText(std::istream& is) {
    Log_N_Debug2_H("")
    Mdn2dIO::locked_loadText(is, *this);
    locked_rebuildMetadata();
    Log_N_Debug2_T("")
}


void mdn::Mdn2dBase::saveBinary(std::ostream& os) const {
    Log_N_Debug_H("");
    auto lock = lockReadOnly();
    locked_saveBinary(os);
    Log_N_Debug_T("");
}


void mdn::Mdn2dBase::locked_saveBinary(std::ostream& os) const {
    Log_N_Debug2_H("")
    Mdn2dIO::locked_saveBinary(*this, os);
    Log_N_Debug2_T("")
}

void mdn::Mdn2dBase::loadBinary(std::istream& is) {
    Log_N_Debug_H("");
    auto lock = lockWriteable();
    locked_loadBinary(is);
    Log_N_Debug_T("");
}


void mdn::Mdn2dBase::locked_loadBinary(std::istream& is) {
    Log_N_Debug2_H("")
    Mdn2dIO::locked_loadBinary(is, *this);
    locked_rebuildMetadata();
    Log_N_Debug2_T("")
}


void mdn::Mdn2dBase::rebuildMetadata() const {
    Log_N_Debug2("");
    auto lock = lockWriteable();
    locked_rebuildMetadata();
}


void mdn::Mdn2dBase::locked_rebuildMetadata() const {
    Log_N_Debug3("");
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
    // m_bounds.min() = {itMinX->first, itMinY->first};
    // m_bounds.max() = {itMaxX->first, itMaxY->first};
}


bool mdn::Mdn2dBase::hasBounds() const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    bool result = locked_hasBounds();
    Log_N_Debug2_T("Result: " << result);
    return result;
}


bool mdn::Mdn2dBase::locked_hasBounds() const {
    bool invalid = !m_bounds.isValid();

    Log_N_Debug3(
        "Bounds: " << m_bounds << ", result: " << (!invalid)
    );
    return !invalid;
}


const mdn::Rect& mdn::Mdn2dBase::bounds() const {
    Log_Debug2_H("");
    auto lock = lockReadOnly();
    const Rect& bounds = locked_bounds();
    If_Log_Showing_Debug2(
        Log_N_Debug2_T("Result: " << bounds);
    );
    return bounds;
}


const mdn::Rect& mdn::Mdn2dBase::locked_bounds() const {
    Log_N_Debug3("Result: " << m_bounds);
    return m_bounds;
}


const std::unordered_map<mdn::Coord, mdn::Digit>&  mdn::Mdn2dBase::data_raw() {
    auto lock = lockReadOnly();
    return locked_data_raw();
}
const std::unordered_map<mdn::Coord, mdn::Digit>&  mdn::Mdn2dBase::locked_data_raw() {
    return m_raw;
}
const std::map<int, mdn::CoordSet>&  mdn::Mdn2dBase::data_xIndex() {
    auto lock = lockReadOnly();
    return locked_data_xIndex();
}
const std::map<int, mdn::CoordSet>&  mdn::Mdn2dBase::locked_data_xIndex() {
    return m_xIndex;
}
const std::map<int, mdn::CoordSet>&  mdn::Mdn2dBase::data_yIndex() {
    auto lock = lockReadOnly();
    return locked_data_yIndex();
}
const std::map<int, mdn::CoordSet>&  mdn::Mdn2dBase::locked_data_yIndex() {
    return m_yIndex;
}
const mdn::CoordSet&  mdn::Mdn2dBase::data_index() {
    auto lock = lockReadOnly();
    return locked_data_index();
}
const mdn::CoordSet&  mdn::Mdn2dBase::locked_data_index() {
    return m_index;
}
const std::unordered_map<int, mdn::MdnObserver*>&  mdn::Mdn2dBase::data_observers() {
    auto lock = lockReadOnly();
    return locked_data_observers();
}
const std::unordered_map<int, mdn::MdnObserver*>&  mdn::Mdn2dBase::locked_data_observers() {
    return m_observers;
}


int mdn::Mdn2dBase::getPrecision() const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    int result = locked_getPrecision();
    Log_N_Debug2_T("result=" << result);
    return result;
}


int mdn::Mdn2dBase::locked_getPrecision() const {
    int result = m_config.precision();
    Log_N_Debug3("Result: " << result);
    return result;
}


int mdn::Mdn2dBase::setPrecision(int newPrecision) {
    Log_N_Debug2_H("New precision: " << newPrecision);
    auto lock = lockWriteable();
    int nDropped = locked_setPrecision(newPrecision);
    internal_operationComplete();
    Log_N_Debug2_T("result=" << nDropped);
    return nDropped;
}


int mdn::Mdn2dBase::locked_setPrecision(int newPrecision) {
    int oldPrecision = m_config.precision();
    m_config.setPrecision(newPrecision);
    Log_N_Debug3("Precision was: " << oldPrecision << ", new value: " << newPrecision);

    if (oldPrecision < newPrecision)
    {
        return internal_purgeExcessDigits();
    }
    return 0;
}


mdn::PrecisionStatus mdn::Mdn2dBase::checkPrecisionWindow(const Coord& xy) const {
    Log_N_Debug3_H("At: " << xy);
    auto lock = lockReadOnly();
    PrecisionStatus result = locked_checkPrecisionWindow(xy);
    If_Log_Showing_Debug3(
        Log_N_Debug3_T("result=" << PrecisionStatusToName(result));
    );
    return result;
}


mdn::PrecisionStatus mdn::Mdn2dBase::locked_checkPrecisionWindow(const Coord& xy) const {
    if (!locked_hasBounds()) {
        Log_N_Debug4("At: " << xy << ", result: Inside (no bounds)");
        return PrecisionStatus::Inside;
    }
    int precision = m_config.precision();

    // minLimit - below this and the new value should not be added
    Coord minLimit = m_bounds.max() - precision;

    // Check under limit
    if (xy.x() < minLimit.x() || xy.y() < minLimit.y()) {
        // Value is under-limit, do not add
        Log_N_Debug4("At: " << xy << ", result: under limit " << minLimit);
        return PrecisionStatus::Below;
    }

    // Check over limit
    CoordSet purgeSet;
    // maxLimit - above this and we need to purge existing values
    Coord maxLimit = m_bounds.min() + precision;
    // Check over limit
    int purgeX = xy.x() - maxLimit.x();
    int purgeY = xy.y() - maxLimit.y();
    if (purgeX > 0 || purgeY > 0) {
        Log_N_Debug4("At: " << xy << ", result: Above (digits need to be purged)");
        return PrecisionStatus::Above;
    }
    Log_N_Debug4("At: " << xy << ", result: Inside");
    return PrecisionStatus::Inside;
}


void mdn::Mdn2dBase::internal_modified() {
    Log_N_Debug4("Modified flag set");
    m_modified = true;
}


void mdn::Mdn2dBase::internal_operationComplete() {
    if (m_modified) {
        Log_N_Debug4("Operation complete, incrementing m_event from " << m_event);
        ++m_event;
        m_modified = false;
    } else {
        Log_N_Debug4("Operation complete, no modifications");
    }
}


void mdn::Mdn2dBase::internal_modifiedAndComplete() {
    Log_N_Debug4("Operation complete and modified, incrementing m_event from " << m_event);
    ++m_event;
    m_modified = false;
}


mdn::Mdn2dBase::WritableLock mdn::Mdn2dBase::lockWriteable() const {
    Log_N_Debug4_H("");
    WritableLock lock = m_lockTracker.lockWriteable(m_mutex);
    Log_N_Debug4_T("returning lock");
    return lock;
}


mdn::Mdn2dBase::ReadOnlyLock mdn::Mdn2dBase::lockReadOnly() const {
    Log_N_Debug4_H("");
    ReadOnlyLock lock = m_lockTracker.lockReadOnly(m_mutex);
    Log_N_Debug4_T("returning lock");
    return lock;
}


void mdn::Mdn2dBase::assertNotSelf(Mdn2dBase& that, const std::string& description) const {
    if (this == &that) {
        IllegalSelfReference err(description);
        Log_N_Error(err.what());
        throw err;
    }
    Log_N_Debug4("Operation okay, not self");
}


void mdn::Mdn2dBase::internal_clearMetadata() const {
    Log_N_Debug3("");
    m_bounds.clear();

    m_xIndex.clear();
    m_yIndex.clear();
    m_index.clear();
}


bool mdn::Mdn2dBase::internal_setValueRaw(const Coord& xy, Digit value) {
    if (value == 0) {
        If_Log_Showing_Debug4(
            Log_N_Debug4_H(
                "Setting " << xy << " to " << static_cast<int>(value) << ": setting to zero"
            );
        );
        locked_setToZero(xy);
        Log_N_Debug4_T("result=0");
        return false;
    }

    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // No entry exists
        If_Log_Showing_Debug4(
            Log_N_Debug4_H(
                "Setting " << xy << " to " << static_cast<int>(value)
                << ", no previous existing value"
            );
        );
        PrecisionStatus ps = locked_checkPrecisionWindow(xy);
        if (ps == PrecisionStatus::Below) {
            // Out of numerical precision range
            Log_N_Debug4_T("New value below precision range, result=0");
            return false;
        }
        internal_modified();
        internal_insertAddress(xy);
        m_raw[xy] = value;
        if (ps == PrecisionStatus::Above) {
            // Above numerical precision range
            Log_N_Debug4("New value above precision range, purging low digits");
            internal_purgeExcessDigits();
            Log_N_Debug4_T("result=1");
            return true;
        }
        If_Log_Showing_Debug4(
            Log_N_Debug4_T("New value within precision range, result=1");
        );
        return true;
    }
    // xy is already non-zero
    Digit oldVal = it->second;
    it->second = value;
    if (oldVal != value) {
        internal_modified();
    } else {
        If_Log_Showing_Debug4(
            Log_N_Debug4(
                "Setting " << xy << " to " << static_cast<int>(value)
                << ", but digit already has that value, result=0"
            );
        );
        return false;
    }

    If_Log_Showing_Debug4(
        Log_N_Debug4_H(
            "Setting " << xy << " to " << static_cast<int>(value)
            << ": overwriting existing value: " << static_cast<int>(oldVal)
        );
    );
    // check for sign change
    bool result = (
        (oldVal < 0 && value > 0) || (oldVal > 0 && value < 0)
    );
    Log_N_Debug4_T("result=" << result);
    return result;
    // Possibly faster
    // return (static_cast<int>(oldVal) * static_cast<int>(value)) < 0;
}


void mdn::Mdn2dBase::internal_insertAddress(const Coord& xy) const {
    Log_N_Debug4("At: " << xy);
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
    m_bounds.growToInclude(xy);
}


int mdn::Mdn2dBase::internal_purgeExcessDigits() {
    Log_N_Debug3_H("");
    if (!locked_hasBounds()) {
        // No digits to bother keeping
        Log_N_Debug3_T("No digits to consider");
        return 0;
    }

    int precision = m_config.precision();
    Coord gridSize = m_bounds.gridSize();
    CoordSet purgeSet;
    int purgeX = gridSize.x() - precision;
    if (purgeX > 0) {
        int minX = m_bounds.max().x() - precision;
        for (const auto& [x, coords] : m_xIndex) {
            if (x >= minX) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    int purgeY = gridSize.y() - precision;
    if (purgeY > 0) {
        int minY = m_bounds.max().y() - precision;
        for (const auto& [y, coords] : m_yIndex) {
            if (y >= minY) break;
            purgeSet.insert(coords.begin(), coords.end());
        }
    }
    if (!purgeSet.empty()) {
        If_Log_Showing_Debug4(
            Log_N_Debug4_H(
                "Purging " << purgeSet.size() << " digits, now below numerical precision window: "
                << m_bounds << ", precision: " << m_config.precision()
            );
        );
        locked_setToZero(purgeSet);
        Log_N_Debug4_T("result=" << purgeSet.size());
        Log_N_Debug3_T("");
        return purgeSet.size();
    }
    Log_N_Debug3_T("No digits purged");
    return 0;
}


void mdn::Mdn2dBase::internal_updateBounds() {
    Log_N_Debug4_H("");
    if (m_xIndex.empty() || m_yIndex.empty()) {
        m_bounds.clear();
        Log_N_Debug3("Updating bounds: no non-zero digits exist, there are no bounds");
    } else {
        auto itMinX = m_xIndex.cbegin();
        auto itMaxX = m_xIndex.crbegin();
        auto itMinY = m_yIndex.cbegin();
        auto itMaxY = m_yIndex.crbegin();
        m_bounds.set(itMinX->first, itMinY->first, itMaxX->first, itMaxY->first);
        Log_N_Debug3("Updating bounds, new bounds: " << m_bounds);
    }
    Log_N_Debug4_T("");
}
