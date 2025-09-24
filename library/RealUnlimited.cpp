#include "RealUnlimited.hpp"


std::string mdn::Tools::stringToArbitraryPrecisionReal(
    bool& negative,
    std::vector<long>& intPart,
    std::vector<long>& fracPart,
    int base,
    const std::string& input
) {
    // base validation
    if (base < 2 || base > 32) {
        return "Invalid input, base out-of-range 2..32, got " + std::to_string(base);
    }

    // reset outputs
    negative = false;
    intPart.clear();
    fracPart.clear();

    // helpers
    auto is_sep   = [](char c) noexcept { return c == '_' || c == '\''; };
    auto is_radix = [](char c) noexcept { return c == '.'; };
    auto to_digit = [base](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
        return -1;
    };

    // strip whitespace
    std::string s;
    s.reserve(input.size());
    for (char c : input) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        s.push_back(c);
    }
    if (s.empty()) {
        return std::string("Invalid input: empty string");
    }

    // sign
    size_t i = 0;
    if (s[i] == '+' || s[i] == '-') {
        negative = (s[i] == '-');
        ++i;
        if (i >= s.size()) {
            return std::string("Invalid input: sign without digits.");
        }
    }

    // split into integer and fractional strings
    std::string intStr, fracStr;
    bool seen_radix = false;
    for (; i < s.size(); ++i) {
        char c = s[i];
        if (is_sep(c)) continue;
        if (is_radix(c)) {
            if (seen_radix) {
                return std::string("Invalid input: multiple radix (e.g. 'decimal') points.");
            }
            seen_radix = true;
            continue;
        }
        int d = to_digit(c);
        if (d < 0 || d >= base) {
            std::ostringstream oss;
            oss << "Invalid input: invalid digit '" << c << "' for base " << base << ".";
            return oss.str();
        }
        (seen_radix ? fracStr : intStr).push_back(c);
    }

    // allow ".xxx" -> "0.xxx"
    if (intStr.empty()) intStr = "0";

    // remove leading zeros in integer part (keep at least one)
    size_t nz = 0;
    while (nz + 1 < intStr.size() && intStr[nz] == '0') ++nz;
    if (nz > 0) intStr.erase(0, nz);

    // helper: convert a slice of base-<base> digits to an integer chunk
    auto chunk_value = [to_digit, base](const std::string& str, size_t pos, size_t len) -> unsigned long long {
        unsigned long long v = 0ULL;
        for (size_t k = 0; k < len; ++k) {
            v = v * static_cast<unsigned long long>(base)
              + static_cast<unsigned long long>(to_digit(str[pos + k]));
        }
        return v; // fits in 64-bit: base^12 <= 2^60 for base <= 32
    };

    // integer part: group from the left, most-significant chunk first
    if (intStr == "0") {
        intPart.push_back(0L);
    } else {
        const size_t n = intStr.size();
        size_t first_len = n % 12;
        if (first_len == 0) first_len = 12;

        size_t pos = 0;
        intPart.push_back(static_cast<long>(chunk_value(intStr, pos, first_len)));
        pos += first_len;

        while (pos < n) {
            intPart.push_back(static_cast<long>(chunk_value(intStr, pos, 12)));
            pos += 12;
        }
    }

    // fractional part: first digits after radix first, 12-digit chunks
    if (!fracStr.empty()) {
        size_t pos = 0, n = fracStr.size();
        while (pos < n) {
            size_t len = std::min<size_t>(12, n - pos);
            fracPart.push_back(static_cast<long>(chunk_value(fracStr, pos, len)));
            pos += len;
        }
    }
    return std::string();
}
