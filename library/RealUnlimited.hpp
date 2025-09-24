#pragma once

#include <vector>
#include <ostream>
#include <sstream>
#include <string>

#include "Logger.hpp"

namespace mdn {

class RealUnlimited {

    // If string constructor encountered an error, m_valid is false and m_invalidReason says why
    bool m_valid;
    std::string m_invalidReason;

    int m_base;

    // chatgpt: You decide how to store 'sign', e.g.:
    bool m_sign;    // true = positive
    int m_sign;     // 1 = positive, -1 = negative, or for these int or long typed sign flags, you
    long m_sign;    // can choose 1=positive, 0=negative.  Mathematically 1 / -1 might be better.

    std::vector<long> m_intPart;
    std::vector<long> m_fracPart;

public:

    // Construct from string - currently the only use-case
    RealUnlimited(const std::string& strIn, int baseIn);

    // Access
    inline bool valid() const { return m_valid; }
    inline const std::string& invalidReason() const { return m_invalidReason; }
    inline std::vector<long>& intPart() { return m_intPart; }
    inline const std::vector<long>& intPart() const { return m_intPart; }
    inline std::vector<long>& fracPart() { return m_fracPart; }
    inline const std::vector<long>& fracPart() const { return m_fracPart; }

    // Utility
    inline void negate() { /* chatgpt: implement this: reverse m_sign */ }

    // Conversions

        // RealUnlimited --> std::string
        operator std::string() const;
        std::string& toString() const;
        friend std::ostream& operator<<(std::ostream& os, const RealUnlimited& ru);

        // std::string --> RealUnlimited
        static RealUnlimited toRealUnlimited(int base, const std::string& input);

    // Other functionality
    // Message to chatgpt
    // Let's keep this simple.  This is really about converting from string to the member data
    // contained herein.  I have a Mdn2d class, which is a multi-dimensional digit number.  I am
    // crafting functions that take this RealUnlimited as an input and fill in the digits in the
    // Mdn2d grid, in the correct positions and everything.
    // In other words, this class does not need any more functionality than what I described above,
    // I think.  No need for mathematical operators (beyond the "negate" function).
};

} // end namespace mdn