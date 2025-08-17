#pragma once

#include <cmath>
#include <sstream>

#include "Constants.h"
#include "Digit.h"
#include "Fraxis.h"
#include "GlobalConfig.h"
#include "Logger.h"
#include "MdnException.h"
#include "Mdn2dFramework.h"
#include "SignConvention.h"

namespace mdn {



// Contains all settings governing behaviour of an Mdn2d
class MDN_API Mdn2dConfig {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero
    // digit within m_precision
    static double static_calculateEpsilon(int precisionIn, int baseIn) {
        return pow((1.0 / baseIn), (precisionIn + 1));
    }

    // Pointer to framework governing class - what object holds this Mdn2d?
    static Mdn2dFramework* m_masterPtr;


public:

    static Mdn2dFramework& getMaster() {
        if (!m_masterPtr) {
            m_masterPtr = &(mdn::DummyFramework);
        }
        return *m_masterPtr;
    }

    // Set master pointer to the given framework, posts warning if already set
    static void setMaster(Mdn2dFramework& framework);

    // Set master pointer to the given framework, overwrites previously existing setting silently
    static void resetMaster(Mdn2dFramework& framework);

    // If a message exists here, the associated Mdn2d is invalid for the reason contained in the
    // string.  Purpose of this is to prevent throwing during move ctor, so normal operation can be
    // optimised. Edge cases that invalidate the number show up here.
    mutable std::string m_invalidReason;

    // Numerical base, beyond which no digit's magnitude can go
    int m_base;

    // Convenience - base expressed as a Digit type
    Digit m_baseDigit;

    // Convenience - base expressed as a double type
    double m_baseDouble;

    // Maximum number of digits from lowest magnitude to maximum magnitude
    int m_precision;

    // Smallest value added to a digit that can cascade to a non-zero value within the precision
    //  window
    double m_epsilon;

    // Default sign convention for polymorphic numbers
    SignConvention m_signConvention;

    // Maximum number of outerloops allowed in attempting to reach polymorphic stability
    //  That is, all required carryovers are done, and polymorphic carryovers meet the default sign
    //  convention.  Use -1 for infinite loops allowed.
    int m_maxCarryoverIters;

    // Affects 1) fractional addition, 2) divide direction
    Fraxis m_defaultFraxis;


public:

    // Summon a purely default Mdn2dConfig object
    static Mdn2dConfig static_defaultConfig() { return Mdn2dConfig(); }


    // *** Constructors

    // Construct from parts, or null
    Mdn2dConfig(
        int baseIn=10,
        int maxSpanIn=16,
        SignConvention signConventionIn=SignConvention::Positive,
        int maxCarryoverItersIn = 20,
        Fraxis defaultFraxisIn=Fraxis::X
    );

    // Returns true if the number became invalid during a noexcept function
    bool valid() { return m_invalidReason.empty(); }

    // Returns false if the number became invalid during a noexcept function
    bool invalid() { return !valid(); }

    // Returns the reason this number became invalid during a noexcept function
    std::string invalidReason() { return m_invalidReason; }

    // Set this number to invalid - required to preserve nexcept optimisations, but also handle edge
    // cases that would have thrown
    void setInvalid(const std::string& reason) { m_invalidReason = reason; }

    // Return the base
    int base() const { return m_base; }

    // Return the base as a Digit type
    Digit baseDigit() const { return m_baseDigit; }

    // Return the base as a double type
    double baseDouble() const { return m_baseDouble; }

    // Return the numeric precision
    int precision() const { return m_precision; }

    // Change the precision, update downstream derived values
    void setPrecision(int precisionIn);

    // Return the derived epsilon value
    double epsilon() const { return m_epsilon; }

    SignConvention signConvention() const { return m_signConvention; }
    void setSignConvention(int newVal);
    void setSignConvention(std::string newName) {
        m_signConvention = NameToSignConvention(newName);
    }
    void setSignConvention(SignConvention signConventionIn) {
        m_signConvention = signConventionIn;
    }

    int maxCarryoverIters() { return m_maxCarryoverIters; }
    void setMaxCarryoverIters(int newVal) {
        m_maxCarryoverIters = newVal < 0 ? constants::intMax : newVal;
    }

    Fraxis defaultFraxis() const { return m_defaultFraxis; }
    void setDefaultFraxis(int newVal);
    void setDefaultFraxis(std::string newName) { m_defaultFraxis = NameToFraxis(newName); }
    void setDefaultFraxis(Fraxis fraxisIn) { m_defaultFraxis = fraxisIn; }

    // Returns true if all settings are valid, false if something failed
    bool checkConfig() const;

    // Throws if any setting is invalid
    void validateConfig() const;

    std::string to_string() const;

    // string format: (b:10, p:16, s:Positive, c:20, f:X)
    friend std::ostream& operator<<(std::ostream& os, const Mdn2dConfig& c) {
        return os
            << "("
                << "b:" << c.m_base
                << ", p:" << c.m_precision
                << ", e:" << c.m_epsilon
                << ", s:" << SignConventionToName(c.m_signConvention)
                << ", c:" << c.m_maxCarryoverIters
                << ", f:" << FraxisToName(c.m_defaultFraxis)
            << ")";
    }

    friend std::istream& operator>>(std::istream& is, Mdn2dConfig& c) {
        // TODO - problem, epsilon is derived from precision, should not be independent
        char lparen, letter, colon, comma, rparen;
        std::string fname;
        std::string sname;
        // (b:10, p:16, e:0.0000024, s:Positive, c:20, f:X)
        // Reading [(b:10]
        is >> lparen >> letter >> colon >> c.m_base;
        // Reading [, p:16]
        is >> comma >> letter >> colon >> c.m_precision;
        // Reading [, e:2.4e-6]
        is >> comma >> letter >> colon >> c.m_epsilon;
        // Reading [, s:Positive,]
        while (letter != ',') {
            is >> letter;
            sname += letter;
        }
        c.m_signConvention = NameToSignConvention(sname);
        //Reading [ c:20]
        is >> letter >> colon >> c.m_maxCarryoverIters;
        //Reading [, f:X])
        while (letter != ')') {
            is >> letter;
            fname += letter;
        }
        c.m_defaultFraxis = NameToFraxis(fname);
        return is;
    }

    // SignConvention m_signConvention;
    // int m_maxCarryoverIters;
    // Fraxis m_defaultFraxis;

        // From the perspective of an Mdn2d, look for compatibility.  The data that matters are:
        //  base, precision, sign convention
        bool operator==(const Mdn2dConfig& rhs) const;
        bool operator!=(const Mdn2dConfig& rhs) const;
};

} // end namespace mdn
