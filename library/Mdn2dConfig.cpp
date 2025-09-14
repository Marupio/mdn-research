#include "Mdn2dConfig.hpp"

#include "Logger.hpp"
#include "MdnException.hpp"


// mdn::Mdn2dFramework* mdn::Mdn2dConfig::m_masterPtr = nullptr;


mdn::Mdn2dFramework& mdn::Mdn2dConfig::master() {
    if (!m_masterPtr) {
        m_masterPtr = &(mdn::DummyFramework);
    }
    return *m_masterPtr;
}


void mdn::Mdn2dConfig::setMaster(Mdn2dFramework& framework) {
    if (m_masterPtr && m_masterPtr->className() != "Mdn2dFramework") {
        Log_Warn(
            "Setting a new framework " << framework.name()
            << ", class " << framework.className()
            << ", when existing framework " << m_masterPtr->name()
            << ", class " << m_masterPtr->className()
            << " already is assigned. Using new framework."
        );
    }
    m_masterPtr = &framework;
    updateIdentity();
}


void mdn::Mdn2dConfig::resetMaster(Mdn2dFramework& framework) {
    m_masterPtr = &framework;
    updateIdentity();
}


mdn::Mdn2dConfig::Mdn2dConfig(
    int baseIn,
    int precisionIn,
    SignConvention signConventionIn,
    int maxCarryoverItersIn,
    Fraxis fraxisIn
) :
    m_base(baseIn),
    m_baseDigit(static_cast<Digit>(baseIn)),
    m_baseDouble(static_cast<double>(baseIn)),
    m_precision(precisionIn),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_signConvention(signConventionIn),
    m_maxCarryoverIters(maxCarryoverItersIn),
    m_fraxis(fraxisIn)
{
    Log_Debug3_H("");
    updateIdentity();
    validateConfig();
    Log_Debug3_T("");
}


void mdn::Mdn2dConfig::updateIdentity() {
    if (m_masterPtr) {
        m_parentName = m_masterPtr->name();
        m_parentPath = m_masterPtr->path();
    } else {
        m_parentName = "";
        m_parentPath = "";
    }
}


void mdn::Mdn2dConfig::setPrecision(int precisionIn) {
    if (precisionIn < 0) {
        throw std::invalid_argument(
            "Got " + std::to_string(precisionIn) + ", precision cannot be less than 1"
        );
    }
    if (m_precision != precisionIn) {
        m_precision = precisionIn;
        m_epsilon = static_calculateEpsilon(m_precision, m_base);
    }
}


void mdn::Mdn2dConfig::setSignConvention(int newVal) {
    if (newVal >= 0 && newVal < SignConventionNames.size()) {
        m_signConvention = static_cast<SignConvention>(newVal);
    } else {
        throw std::invalid_argument(
            "SignConvention must be a value between 0 and " +
            std::to_string(SignConventionNames.size()) +
            ", got " + std::to_string(newVal)
        );
    }
}


void mdn::Mdn2dConfig::setFraxis(int newVal) {
    if (newVal >= 0 && newVal < FraxisNames.size()) {
        m_fraxis = static_cast<Fraxis>(newVal);
    } else {
        throw std::invalid_argument(
            "Fraxis must be a value between 0 and " + std::to_string(FraxisNames.size()) +
            ", got " + std::to_string(newVal)
        );
    }
}


bool mdn::Mdn2dConfig::checkConfig() const {
    return (
        (m_base >= 2 && m_base <= 32) &&
        (m_precision > 0) &&
        (
            (m_signConvention == SignConvention::Positive) ||
            (m_signConvention == SignConvention::Negative) ||
            (m_signConvention == SignConvention::Neutral)
        ) &&
        ((m_fraxis == Fraxis::X) || (m_fraxis == Fraxis::Y))
    );
}


void mdn::Mdn2dConfig::validateConfig() const {
    Log_Debug3_H("");
    if (!checkConfig()) {
        std::ostringstream oss;
        oss << "Mdn2dConfig has an invalid setting:" << std::endl;
        oss << "    base = " << m_base << ", expecting be 2 .. 32" << std::endl;
        oss << "    precision = " << m_precision << ", must be > 0" << std::endl;
        oss << "    signConvention = " << SignConventionToName(m_signConvention);
        oss << ", expecting: 'Neutral', 'Positive', or 'Negative'" << std::endl;
        oss << "    maxCarryoverIters = " << m_maxCarryoverIters << std::endl;
        oss << "    fraxis = " << FraxisToName(m_fraxis)
            << ", expecting 'X' or 'Y'";
        InvalidArgument err(oss.str());
        Log_Error(err.what());
        throw err;
    }
    Log_Debug3_T("");
}


std::string mdn::Mdn2dConfig::toString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}


bool mdn::Mdn2dConfig::operator==(const Mdn2dConfig& rhs) const {
    bool result = (
        rhs.m_base == m_base &&
        rhs.m_precision == m_precision &&
        rhs.m_signConvention == m_signConvention &&
        rhs.m_maxCarryoverIters == m_maxCarryoverIters &&
        rhs.m_fraxis == m_fraxis
    );
    If_Log_Showing_Debug3(
        Log_Debug3(
            "rhs=" << rhs << ", lhs=" << *this
        );
    );
    return result;
}


bool mdn::Mdn2dConfig::operator!=(const Mdn2dConfig& rhs) const {
    return !(rhs == *this);
}
