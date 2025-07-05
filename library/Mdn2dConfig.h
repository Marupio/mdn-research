#pragma once

#include <sstream>

#include "Digit.h"
#include "Fraxis.h"
#include "MdnException.h"
#include "SignConvention.h"

namespace mdn {

// Contains all settings governing behaviour of an Mdn2d
class Mdn2dConfig {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero
    // digit within m_precision
    static double static_calculateEpsilon(int precisionIn, int baseIn) {
        return pow((1.0 / baseIn), (precisionIn + 1));
    }

    // Numerical base, beyond which no digit's magnitude can go
    int m_base;

    // Convenience - base expressed as a Digit type
    Digit m_dbase;

    // Maximum number of digits from lowest magnitude to maximum magnitude
    int m_precision;

    // Smallest value added to a digit that can cascade to a non-zero value within the precision
    //  window
    int m_epsilon;

    // Default sign convention for polymorphic numbers
    SignConvention m_defaultSignConvention;

    // Affects 1) fractional addition, 2) divide direction
    Fraxis m_defaultFraxis;


public:

    // Summon a purely default Mdn2dConfig object
    static Mdn2dConfig static_defaultConfig() { return Mdn2dConfig(); }


    // *** Constructors

    // Construct from parts
    Mdn2dConfig(
        int baseIn=10,
        int maxSpanIn=16,
        SignConvention defaultSignConventionIn=SignConvention::Positive,
        Fraxis defaultFraxisIn=Fraxis::X
    ) :
        m_base(baseIn),
        m_precision(maxSpanIn),
        m_epsilon(static_calculateEpsilon(m_precision, m_base)),
        m_defaultSignConvention(defaultSignConventionIn),
        m_defaultFraxis(defaultFraxisIn)
    {}

    // Return the base
    int base() const { return m_base; }

    // Return the base as a Digit type
    Digit dbase() const { return m_dbase; }

    // Return the numeric precision
    int precision() const { return m_precision; }

    // Change the precision, update downstream derived values
    void setPrecision(int precisionIn) {
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

    // Return the derived epsilon value
    int epsilon() const { return m_epsilon; }

    SignConvention defaultSignConvention() const { return m_defaultSignConvention; }
    void setDefaultSignConvention(int newVal) {
        if (newVal >= 0 && newVal < SignConventionNames.size()) {
            m_defaultSignConvention = static_cast<SignConvention>(newVal);
        } else {
            throw std::invalid_argument(
                "SignConvention must be a value between 0 and " +
                std::to_string(SignConventionNames.size()) +
                ", got " + std::to_string(newVal)
            );
        }
    }
    void setDefaultSignConvention(std::string newName) {
        m_defaultSignConvention = NameToSignConvention(newName);
    }
    void setDefaultSignConvention(SignConvention signConventionIn) {
        m_defaultSignConvention = signConventionIn;
    }

    Fraxis defaultFraxis() const { return m_defaultFraxis; }
    void setDefaultFraxis(int newVal) {
        if (newVal >= 0 && newVal < FraxisNames.size()) {
            m_defaultFraxis = static_cast<Fraxis>(newVal);
        } else {
            throw std::invalid_argument(
                "Fraxis must be a value between 0 and " + std::to_string(FraxisNames.size()) +
                ", got " + std::to_string(newVal)
            );
        }
    }
    void setDefaultFraxis(std::string newName) { m_defaultFraxis = NameToFraxis(newName); }
    void setDefaultFraxis(Fraxis fraxisIn) { m_defaultFraxis = fraxisIn; }

    // Returns true if all settings are valid, false if something failed
    bool checkConfig() const {
        return (
            (m_base >= 2 && m_base <= 32) &&
            (m_precision > 0) &&
            (
                (m_defaultSignConvention == SignConvention::Positive) ||
                (m_defaultSignConvention == SignConvention::Negative) ||
                (m_defaultSignConvention == SignConvention::Default)
            ) &&
            ((m_defaultFraxis == Fraxis::X) || (m_defaultFraxis == Fraxis::Y))
        );
    }

    // Throws if any setting is invalid
    void validateConfig() const {
        if (!checkConfig()) {
            std::ostringstream oss;
            oss << "Mdn2dConfig has an invalid setting:" << std::endl;
            oss << "    base = " << m_base << ", expecting be 2 .. 32" << std::endl;
            oss << "    maxSpan = " << m_precision << ", must be > 0" << std::endl;
            oss << "    defaultFraxis = " << FraxisToName(m_defaultFraxis)
                << ", expecting 'X' or 'Y'";
            throw std::invalid_argument(oss.str());
        }
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    // string format: (b:10, p:16, f:X)
    friend std::ostream& operator<<(std::ostream& os, const Mdn2dConfig& c) {
        return os << "(b:" << c.m_base << ", p:" << c.m_precision << ", f:"
            << FraxisToName(c.m_defaultFraxis) << ")";
    }

    friend std::istream& operator>>(std::istream& is, Mdn2dConfig& c) {
        char lparen, letter, colon, comma, rparen;
        std::string fname;
        is >> lparen >> letter >> colon >> c.m_base;
        is >> comma >> letter >> colon >> c.m_precision;
        while (letter != ')') {
            is >> letter;
            fname += letter;
        }
        c.m_defaultFraxis = NameToFraxis(fname);
        return is;
    }

};

} // end namespace mdn
