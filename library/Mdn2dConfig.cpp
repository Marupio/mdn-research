#include "Mdn2dConfig.h"


mdn::Mdn2dFramework* mdn::Mdn2dConfig::m_masterPtr = nullptr;


void mdn::Mdn2dConfig::setMaster(Mdn2dFramework& framework) {
    if (m_masterPtr != &DummyFramework) {
        Log_Warn(
            "Setting a new framework " << framework.name()
            << ", class " << framework.className()
            << ", when existing framework " << framework.name()
            << ", class " << framework.className()
            << " already is assigned. Using new framework."
        );
    }
    m_masterPtr = &framework;
}


void mdn::Mdn2dConfig::resetMaster(Mdn2dFramework& framework) {
    m_masterPtr = &framework;
}


mdn::Mdn2dConfig::Mdn2dConfig(
    int baseIn,
    int precisionIn,
    SignConvention signConventionIn,
    int maxCarryoverItersIn,
    Fraxis defaultFraxisIn
) :
    m_base(baseIn),
    m_baseDigit(static_cast<Digit>(baseIn)),
    m_baseDouble(static_cast<double>(baseIn)),
    m_precision(precisionIn),
    m_epsilon(static_calculateEpsilon(m_precision, m_base)),
    m_signConvention(signConventionIn),
    m_maxCarryoverIters(maxCarryoverItersIn),
    m_defaultFraxis(defaultFraxisIn)
{
    validateConfig();
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


void mdn::Mdn2dConfig::setDefaultFraxis(int newVal) {
    if (newVal >= 0 && newVal < FraxisNames.size()) {
        m_defaultFraxis = static_cast<Fraxis>(newVal);
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
            (m_signConvention == SignConvention::Default)
        ) &&
        ((m_defaultFraxis == Fraxis::X) || (m_defaultFraxis == Fraxis::Y))
    );
}


void mdn::Mdn2dConfig::validateConfig() const {
    if (!checkConfig()) {
        std::ostringstream oss;
        oss << "Mdn2dConfig has an invalid setting:" << std::endl;
        oss << "    base = " << m_base << ", expecting be 2 .. 32" << std::endl;
        oss << "    precision = " << m_precision << ", must be > 0" << std::endl;
        oss << "    signConvention = " << SignConventionToName(m_signConvention);
        oss << ", expecting: 'Default', 'Positive', or 'Negative'" << std::endl;
        oss << "    maxCarryoverIters = " << m_maxCarryoverIters << std::endl;
        oss << "    defaultFraxis = " << FraxisToName(m_defaultFraxis)
            << ", expecting 'X' or 'Y'";
        throw std::invalid_argument(oss.str());
    }
}


std::string mdn::Mdn2dConfig::to_string() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}


bool mdn::Mdn2dConfig::operator==(const Mdn2dConfig& rhs) const {
    return (
        rhs.m_base == m_base &&
        rhs.m_precision == m_precision &&
        rhs.m_signConvention == m_signConvention
    );
}


bool mdn::Mdn2dConfig::operator!=(const Mdn2dConfig& rhs) const {
    return !(rhs == *this);
}
