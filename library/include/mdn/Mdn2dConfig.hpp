#pragma once

#include <sstream>

#include <mdn/Constants.hpp>
#include <mdn/Digit.hpp>
#include <mdn/Fraxis.hpp>
#include <mdn/GlobalConfig.hpp>
#include <mdn/Logger.hpp>
#include <mdn/MdnException.hpp>
#include <mdn/Mdn2dFramework.hpp>
#include <mdn/SignConvention.hpp>

namespace mdn {



// Contains all settings governing behaviour of an Mdn2d
class MDN_API Mdn2dConfig {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero
    // digit within m_precision.  Returns 0 for unlimited precision.
    static double static_calculateEpsilon(int precisionIn, int baseIn);

    // Pointer to framework governing class - what object holds this Mdn2d?
    // static Mdn2dFramework* m_parentPtr;
    Mdn2dFramework* m_parentPtr;


public:

    // Return default fraxisCascadeDepth
    static int defaultFraxisCascadeDepth() {
        return 20;
    }

    // Return framework parent, controls Mdn2d naming
    Mdn2dFramework& parent();

    // Set parent pointer to the given framework, posts warning if already set
    //  * Note - ensure parent has up-to-date 'name' and 'path' set first
    void setParent(Mdn2dFramework& framework);

    // Set parent pointer to the given framework, overwrites previously existing setting silently
    //  * Note - ensure parent has up-to-date 'name' and 'path' set first
    void resetParent(Mdn2dFramework& framework);

    // Sets m_parentName and m_parentPath based on current parent
    void updateIdentity();


private:

    // Private member data

    // If a message exists here, the associated Mdn2d is invalid for the reason contained in the
    // string.  Purpose of this is to prevent throwing during move ctor, so normal operation can be
    // optimised. Edge cases that invalidate the number show up here.
    mutable std::string m_invalidReason;

    // Name of the framework parent, if the parent is present
    //  Does not affect operator== or operator!=
    std::string m_parentName;

    // Path to the framework parent's file
    //  Only set if:
    //      * framework parent exists
    //      * framework parent was saved or loaded recently
    //  Does not affect operator== or operator!=
    std::string m_parentPath;

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
    int m_fraxisCascadeDepth;

    // Affects 1) fractional addition, 2) divide direction
    Fraxis m_fraxis;


public:

    // Summon a purely default Mdn2dConfig object
    static Mdn2dConfig static_defaultConfig() { return Mdn2dConfig(); }


    // *** Constructors

    // Construct from parts, or null
    Mdn2dConfig(
        int baseIn=10,
        int precisionIn=-1,
        SignConvention signConventionIn=SignConvention::Positive,
        int fraxisCascadeDepthIn = 20,
        Fraxis fraxisIn=Fraxis::X,
        Mdn2dFramework* parent = nullptr
    );

    // Return parent name
    inline const std::string& parentName() const { return m_parentName; }

    // Set parent name
    inline void setParentName(const std::string& nameIn) { m_parentName = nameIn; }

    // Return parent path
    inline const std::string& parentPath() const { return m_parentPath; }

    // Set parent path
    inline void setParentPath(const std::string& pathIn) { m_parentPath = pathIn; }

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

    // Return the base
    void setBase(int base) {
        m_base = base;
        m_baseDigit = static_cast<Digit>(m_base);
        m_baseDigit = static_cast<double>(m_base);
    }

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

    int fraxisCascadeDepth() const { return m_fraxisCascadeDepth; }
    bool fraxisCascadeDepthIsDefault() const {
        return m_fraxisCascadeDepth == defaultFraxisCascadeDepth();
    }
    void setFraxisCascadeDepth(int newVal) {
        m_fraxisCascadeDepth = newVal < 0 ? constants::intMax : newVal;
    }

    Fraxis fraxis() const { return m_fraxis; }
    void setFraxis(int newVal);
    void setFraxis(std::string newName) { m_fraxis = NameToFraxis(newName); }
    void setFraxis(Fraxis fraxisIn) { m_fraxis = fraxisIn; }

    // Returns true if all settings are valid, false if something failed
    bool checkConfig() const;

    // Throws if any setting is invalid
    void validateConfig() const;

    // Brings in any settings changes from cfg, keeps parent and identity unchanged
    void update(const Mdn2dConfig& cfg);

    // Convert to string
    std::string toString() const;

    // string format: (b:10, p:16, s:Positive, c:20, f:X)
    // Does not output parent name / parent path
    friend std::ostream& operator<<(std::ostream& os, const Mdn2dConfig& c) {
        return os
            << "("
                << "b:" << c.m_base
                << ", p:" << c.m_precision
                // << ", e:" << c.m_epsilon
                << ", s:" << c.m_signConvention
                << ", c:" << c.m_fraxisCascadeDepth
                << ", f:" << FraxisToName(c.m_fraxis)
            << ")";
    }

    friend std::istream& operator>>(std::istream& is, Mdn2dConfig& c) {
        // string format: (b:10, p:16, s:Positive, c:20, f:X)
        char lparen, letter, colon, comma, rparen;
        std::string fname;
        std::string sname;

        // Reading [(b:10]
        is >> lparen >> letter >> colon >> c.m_base;

        // Reading [, p:16]
        is >> comma >> letter >> colon >> c.m_precision;

        // Reading [, s:Positive,]
        is >> comma >> letter >> colon;
        is >> letter;
        do {
            sname += letter;
            is >> letter;
        } while (letter != ',');
        c.m_signConvention = NameToSignConvention(sname);

        //Reading [ c:20]
        is >> letter >> colon >> c.m_fraxisCascadeDepth;

        //Reading [, f:X])
        is >> comma >> letter >> colon;
        is >> letter;
        do {
            fname += letter;
            is >> letter;
        } while (letter != ')');
        c.m_fraxis = NameToFraxis(fname);

        // Done reading
        c.m_epsilon = static_calculateEpsilon(c.m_precision, c.m_base);
        return is;
    }

    // SignConvention m_signConvention;
    // int m_fraxisCascadeDepth;
    // Fraxis m_fraxis;

        // From the perspective of an Mdn2d, look for compatibility
        //  Does not compare parent, name or path
        bool operator==(const Mdn2dConfig& rhs) const;
        bool operator!=(const Mdn2dConfig& rhs) const;
};

} // end namespace mdn
