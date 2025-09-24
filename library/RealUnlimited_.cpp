#include "RealUnlimited.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace mdn {

RealUnlimited::RealUnlimited()
    : m_valid(false),
      m_invalidReason(),
      m_negative(false),
      m_base(10),
      m_intPart(),
      m_fracPart()
{
}

RealUnlimited::RealUnlimited(int base, const std::string& input)
    : m_valid(false),
      m_invalidReason(),
      m_negative(false),
      m_base(base),
      m_intPart(),
      m_fracPart()
{
    bool neg = false;
    std::vector<long> ip;
    std::vector<long> fp;
    std::string reason;

    if (parse(base, input, neg, ip, fp, reason)) {
        m_valid = true;
        m_negative = neg;
        m_intPart = std::move(ip);
        m_fracPart = std::move(fp);
    } else {
        m_valid = false;
        m_invalidReason = reason;
        m_negative = false;
        m_intPart.clear();
        m_fracPart.clear();
        Log_Error(
            "RealUnlimited parse failed: '" << input
            << "' base=" << base
            << " reason=" << m_invalidReason
        );
    }
}

RealUnlimited RealUnlimited::toRealUnlimited(int base, const std::string& input)
{
    return RealUnlimited(base, input);
}

bool RealUnlimited::valid() const
{
    return m_valid;
}

const std::string& RealUnlimited::invalidReason() const
{
    return m_invalidReason;
}

bool RealUnlimited::negative() const
{
    return m_negative;
}

const std::vector<long>& RealUnlimited::intPart() const
{
    return m_intPart;
}

const std::vector<long>& RealUnlimited::fracPart() const
{
    return m_fracPart;
}

int RealUnlimited::base() const
{
    return m_base;
}

void RealUnlimited::negate()
{
    m_negative = !m_negative;
}

// static
bool RealUnlimited::validateBase(int base)
{
    return base >= kMinBase && base <= kMaxBase;
}

// static
bool RealUnlimited::isSeparator(char c)
{
    return c == '_' || c == '\'';
}

// static
bool RealUnlimited::isRadix(char c)
{
    return c == '.';
}

// static
bool RealUnlimited::isWhitespace(char c)
{
    switch (c) {
        case ' ':  return true;
        case '\t': return true;
        case '\n': return true;
        case '\r': return true;
        default:   return false;
    }
}

// static
void RealUnlimited::stripWhitespace(const std::string& in, std::string& out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (!isWhitespace(in[i])) {
            out.push_back(in[i]);
        }
    }
}

// static
int RealUnlimited::charToDigit(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<int>(c - '0');
    }
    if (c >= 'a' && c <= 'z') {
        return 10 + static_cast<int>(c - 'a');
    }
    if (c >= 'A' && c <= 'Z') {
        return 10 + static_cast<int>(c - 'A');
    }
    return -1;
}

// static
unsigned long long RealUnlimited::chunkValue(
    const std::string& str,
    std::size_t pos,
    std::size_t len,
    int base
) {
    unsigned long long v = 0ULL;
    for (std::size_t k = 0; k < len; ++k) {
        const int d = charToDigit(str[pos + k]);
        v = v * static_cast<unsigned long long>(base)
          + static_cast<unsigned long long>(d);
    }
    return v;
}

// static
bool RealUnlimited::parse(
    int base,
    const std::string& input,
    bool& negative,
    std::vector<long>& intPart,
    std::vector<long>& fracPart,
    std::string& reason
) {
    negative = false;
    intPart.clear();
    fracPart.clear();
    reason.clear();

    if (!validateBase(base)) {
        std::ostringstream oss;
        oss << "base out-of-range 2..32, got " << base;
        reason = oss.str();
        return false;
    }

    std::string s;
    stripWhitespace(input, s);
    if (s.empty()) {
        reason = "empty input";
        return false;
    }

    std::size_t i = 0;
    if (s[i] == '+' || s[i] == '-') {
        negative = (s[i] == '-');
        ++i;
        if (i >= s.size()) {
            reason = "sign without digits";
            return false;
        }
    }

    std::string intStr;
    std::string fracStr;
    bool seenRadix = false;

    for (; i < s.size(); ++i) {
        const char c = s[i];
        if (isSeparator(c)) {
            continue;
        }
        if (isRadix(c)) {
            if (seenRadix) {
                reason = "multiple radix points";
                return false;
            }
            seenRadix = true;
            continue;
        }
        const int d = charToDigit(c);
        if (d < 0 || d >= base) {
            std::ostringstream oss;
            oss << "invalid digit '" << c << "' for base " << base;
            reason = oss.str();
            return false;
        }
        if (seenRadix) {
            fracStr.push_back(c);
        } else {
            intStr.push_back(c);
        }
    }

    if (intStr.empty()) {
        intStr = "0";
    }

    // Trim leading zeros in integer part but keep at least one
    std::size_t nz = 0;
    while ((nz + 1) < intStr.size() && intStr[nz] == '0') {
        ++nz;
    }
    if (nz > 0) {
        intStr.erase(0, nz);
    }

    // Build integer chunks from most significant side
    if (intStr == "0") {
        intPart.push_back(0L);
    } else {
        const std::size_t n = intStr.size();
        std::size_t firstLen = n % kChunkDigits;
        if (firstLen == 0) {
            firstLen = kChunkDigits;
        }
        std::size_t pos = 0;
        unsigned long long v = chunkValue(intStr, pos, firstLen, base);
        intPart.push_back(static_cast<long>(v));
        pos += firstLen;
        while (pos < n) {
            v = chunkValue(intStr, pos, kChunkDigits, base);
            intPart.push_back(static_cast<long>(v));
            pos += kChunkDigits;
        }
    }

    // Build fractional chunks from the radix forward
    if (!fracStr.empty()) {
        std::size_t pos = 0;
        const std::size_t n = fracStr.size();
        while (pos < n) {
            const std::size_t len =
                (n - pos >= kChunkDigits) ? kChunkDigits : (n - pos);
            unsigned long long v = chunkValue(fracStr, pos, len, base);
            fracPart.push_back(static_cast<long>(v));
            pos += len;
        }
    }

    return true;
}

} // namespace mdn
