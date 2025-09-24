#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Logger.hpp"

namespace mdn {

class RealUnlimited {
public:
    // Constructors
    RealUnlimited();
    RealUnlimited(int base, const std::string& input);

    // Factory: parse without throwing; check valid() / invalidReason() after.
    static RealUnlimited toRealUnlimited(int base, const std::string& input);

    // Observers
    bool valid() const;
    const std::string& invalidReason() const;
    bool negative() const;
    const std::vector<long>& intPart() const;
    const std::vector<long>& fracPart() const;
    int base() const;

    // Mutators
    void negate();

private:
    // Storage
    bool m_valid;
    std::string m_invalidReason;
    bool m_negative;
    int m_base;
    std::vector<long> m_intPart;
    std::vector<long> m_fracPart;

    // Constants
    static const int kMinBase = 2;
    static const int kMaxBase = 32;
    static const std::size_t kChunkDigits = 12;

    // Parsing helpers (declared in header for discoverability / reuse)
    static bool parse(
        int base,
        const std::string& input,
        bool& negative,
        std::vector<long>& intPart,
        std::vector<long>& fracPart,
        std::string& reason
    );
    static bool validateBase(int base);
    static bool isSeparator(char c);
    static bool isRadix(char c);
    static bool isWhitespace(char c);
    static void stripWhitespace(const std::string& in, std::string& out);
    static int charToDigit(char c);
    static unsigned long long chunkValue(
        const std::string& str,
        std::size_t pos,
        std::size_t len,
        int base
    );
};

} // end namespace mdn
