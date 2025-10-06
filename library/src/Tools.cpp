#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <limits>

#include <mdn/Tools.hpp>
#include <mdn/Logger.hpp>

const std::vector<char> mdn::Tools::m_digToAlphaUpper(
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V',
    }
);
const std::vector<char> mdn::Tools::m_digToAlphaLower(
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v',
    }
);
const std::vector<char> mdn::Tools::m_digToAlphaDisplay(
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'L', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v',
    }
);

const std::string mdn::Tools::BoxArtStr_h = u8"\u2500"; // ─
const std::string mdn::Tools::BoxArtStr_v = u8"\u2502"; // │
const std::string mdn::Tools::BoxArtStr_x = u8"\u253C"; // ┼


namespace { // anonymous

std::string prefixSpacesToWidth(const std::string& input, int width) {
    if (width <= 0 || input.size() >= static_cast<std::size_t>(width)) {
        return input;
    }

    const auto w = static_cast<std::size_t>(width);
    std::string out(w, ' ');
    std::copy(input.begin(), input.end(), out.begin() + (w - input.size()));
    return out;
}

} // end anonymous namespace

std::string mdn::Tools::digitToAlpha(
    Digit value,
    bool alphaNumerics,
    std::string pos,
    std::string neg,
    int padSpacesToWidth
) {
    std::string prefix;
    if (value < 0) {
        prefix += neg;
        value = -value;
    } else {
        prefix += pos;
    }
    std::string ret;
    if (value >= 0 && value <= 31) {
        if (alphaNumerics) {
            ret = prefix + m_digToAlphaDisplay[value];
        } else {
            // May have to break value into pieces, maximum two digits
            int intVal = static_cast<int>(value);
            static const int ten = 10;
            int tens = intVal/ten;
            int ones = intVal%ten;
            if (tens == 0) {
                if (padSpacesToWidth) {
                    ret = " " + prefix + std::to_string(ones);
                } else {
                    ret = prefix + std::to_string(ones);
                }
            } else {
                ret = prefix + std::to_string(tens) + std::to_string(ones);
            }
        }
    } else {
    #ifdef MDN_DEBUG
        throw std::out_of_range(
            "digitToAlpha: value out of range (0..31): " + std::to_string(value)
        );
    #else
        ret = "??"; // fallback for release builds
    #endif
    }
    return prefixSpacesToWidth(ret, padSpacesToWidth);
}


std::string mdn::Tools::digitToAlpha(
    int value,
    bool alphaNumerics,
    std::string pos,
    std::string neg,
    int padSpacesToWidth
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg, padSpacesToWidth);
}


std::string mdn::Tools::digitToAlpha(
    long value,
    bool alphaNumerics,
    std::string pos,
    std::string neg,
    int padSpacesToWidth
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg, padSpacesToWidth);
}


std::string mdn::Tools::digitToAlpha(
    long long value,
    bool alphaNumerics,
    std::string pos,
    std::string neg,
    int padSpacesToWidth
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg, padSpacesToWidth);
}


std::string mdn::Tools::digitArrayToString(
    const std::vector<Digit>& array,
    char delim,
    char open,
    char close
) {
    std::ostringstream oss;
    if (array.size()) {
        oss << open << array[0];
        for (auto citer = array.cbegin()+1; citer != array.cend(); ++citer) {
            oss << ", " << *citer;
        }
        oss << close;
    } else {
        oss << open << close;
    }
    return oss.str();
}


int mdn::Tools::unsafe_alphaToDigit(char c) {
    if (c <= '9' && c >= '0') {
        return int(c - '0');
    }
    return int(c - 'A' + 10);
}


void mdn::Tools::stabilise(float& div) {
    if (div < 0) {
        if (div > -constants::floatSmall) {
            div = -constants::floatSmall;
        }
    } else if (div  < constants::floatSmall) {
        div = constants::floatSmall;
    }
}


void mdn::Tools::stabilise(double& div) {
    if (div < 0) {
        if (div > -constants::doubleSmall) {
            div = -constants::doubleSmall;
        }
    } else if (div  < constants::doubleSmall) {
        div = constants::doubleSmall;
    }
}


std::string mdn::Tools::removePath(const char* fullpath) {
    return std::filesystem::path(fullpath).filename().string();
}


std::pair<std::string, int> mdn::Tools::strInt(const std::string& inStr) {
    int nChar = inStr.size();
    if (!isdigit(inStr[nChar-1])) {
        return std::pair<std::string, int>(inStr, -1);
    }
    int cursor = nChar - 1;
    while (cursor >= 0 && isdigit(inStr[cursor])) {cursor--;}
    cursor++;
    std::string valStr = inStr.substr(cursor, nChar - cursor);
    int val = std::stoi(valStr);
    std::string pre = inStr.substr(0, cursor);
    return std::pair<std::string, int>(pre, val);
}


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

    if (!fracStr.empty()) {
        // Build fractional digits as base-B digits (most-significant first)
        std::vector<int> fracDigits;
        fracDigits.reserve(fracStr.size());
        for (char c : fracStr) {
            fracDigits.push_back(to_digit(c));
        }

        // Optional normalisation: drop trailing zeros in the input fractional digits only.
        // (These don't change the value and makes ".1" == ".10", as desired.)
        while (!fracDigits.empty() && fracDigits.back() == 0) {
            fracDigits.pop_back();
        }

        if (!fracDigits.empty()) {
            // Convert by repeatedly multiplying the base-B fractional vector by 10^12
            // and harvesting the integer carry as the next decimal chunk.
            static const unsigned long long DEC_CHUNK = 1000000000000ULL; // 10^12

            // Guard for non-terminating decimal expansions when base has primes other than 2 or 5.
            // For such bases the decimal expansion repeats; we don't want an infinite loop.
            // We detect repetition of the fractional state.
            std::unordered_set<std::string> seen;
            auto stateKey = [&](){
                std::string key;
                key.resize(fracDigits.size());
                for (size_t i = 0; i < fracDigits.size(); ++i) {
                    key[i] = static_cast<char>(fracDigits[i]); // small ints 0..31 fit
                }
                return key;
            };

            while (!fracDigits.empty()) {
                // Stop if this fractional state repeats
                // (decimal expansion would be non-terminating)
                std::string key = stateKey();
                if (!seen.insert(key).second) {
                    // Non-terminating decimal expansion for this base and digit tail.
                    // You can choose to log and break; here we stop emitting further chunks.
                    // (Or you could set a validity flag / reason in your class.)
                    break;
                }

                // Multiply the fractional tail by 10^12 in base-B digits.
                unsigned long long carry = 0ULL;
                for (int idx = static_cast<int>(fracDigits.size()) - 1; idx >= 0; --idx) {
                    unsigned long long x =
                        static_cast<unsigned long long>(fracDigits[idx]) * DEC_CHUNK + carry;
                    fracDigits[idx] = static_cast<int>(x % static_cast<unsigned long long>(base));
                    carry = x / static_cast<unsigned long long>(base);
                }

                // 'carry' is the next 12-decimal-digit chunk
                fracPart.push_back(static_cast<long>(carry));

                // Trim any zeros at the end of the fractional tail so we eventually terminate
                // when the fraction truly terminates in decimal
                // (e.g., bases with factors 2 and/or 5).
                while (!fracDigits.empty() && fracDigits.back() == 0) {
                    fracDigits.pop_back();
                }
            }

            // Optional: If you *want* to display as minimal integers (e.g., show 5 instead of
            // 500000000000 for base-2 0.1), you can post-process when formatting by stripping
            // trailing decimal zeros only for the LAST chunk you print. Do NOT mutate storage.
        }


        // // fractional part: first digits after radix first, 12-digit chunks
        // if (!fracStr.empty()) {
        //     size_t pos = 0, n = fracStr.size();
        //     while (pos < n) {
        //         size_t len = std::min<size_t>(12, n - pos);
        //         fracPart.push_back(static_cast<long>(chunk_value(fracStr, pos, len)));
        //         pos += len;
        //     }
        // }
    }
    return std::string();
}


constexpr bool mdn::Tools::validUppercaseDigit(const char digit, int base) noexcept {
    if (base < 2 || base > 32) return false;

    if (digit >= '0' && digit <= '9') {
        return (digit - '0') < base;
    }

    if (digit >= 'A' && digit <= 'Z') {
        if (base <= 10) {
            // letters only when base > 10
            return false;
        }
        // 'A'==10, 'B'==11, ...
        return (10 + (digit - 'A')) < base;
    }

    return false;
}


std::string mdn::Tools::verifyStringAsReal(
    const std::string& strIn, int base,
    bool& negative, std::string& intPart, std::string& fracPart, int& radix
) {
    Log_Debug3_H("strIn=[" << strIn << "],b=" << base);
    std::string working = strIn;
    std::transform(working.begin(), working.end(), working.begin(), toupper);
    int cursor = 0;
    negative = false;
    if (working[cursor] == '+') {
        ++cursor;
    } else if (working[cursor] == '-') {
        ++cursor;
        negative = true;
    }
    radix = -1;
    bool encounteredDigit = false;
    while (cursor < working.size()) {
        char c = working[cursor];
        if (c == '.') {
            radix = cursor++;
            break;
        } else if (validUppercaseDigit(c, base)) {
            encounteredDigit = true;
            intPart += c;
        } else {
            std::ostringstream oss;
            oss << "Invalid digit '" << c << "' at " << cursor;
            Log_Warn(oss.str());
            Log_Debug3_T("");
            return oss.str();
        }
        ++cursor;
    }
    // Radix encountered, or we are done
    while (cursor < working.size()) {
        char c = working[cursor];
        if (validUppercaseDigit(c, base)) {
            fracPart += c;
        } else {
            std::ostringstream oss;
            oss << "Invalid digit '" << c << "' at " << cursor;
            Log_Warn(oss.str());
            Log_Debug3_T("");
            return oss.str();
        }
        ++cursor;
    }
    if (!(intPart.size() + fracPart.size())) {
        std::string result("Does not contain any valid digits.");
        Log_Warn(result);
        Log_Debug3_T("");
        return result;
    }
    // A valid number
    Log_Debug3_T("valid");
    return "";
}


double mdn::Tools::ulp(double x) {
    if (!std::isfinite(x)) {
        return std::numeric_limits<double>::infinity();
    }
    double ax = std::fabs(x);
    double next = std::nextafter(ax, std::numeric_limits<double>::infinity());
    return next - ax; // >= 0
}


std::pair<int, int> mdn::Tools::significanceBand(long double x, int base) {
    // if (base < 2 || base > 32) throw std::invalid_argument("base in [2,32]");
    long double value = std::fabs(x);
    if (value == 0.0) {
        // just "0"
        return {0, 0};
    }

    // half-ULP absolute uncertainty
    const long double delta = 0.5 * ulp(value);
    const long double lb = std::log(static_cast<long double>(base));
    const int kmax = static_cast<int>(std::floor(std::log(value) / lb));
    int kleast = static_cast<int>(std::floor(std::log(delta) / lb) + 1);
    if (kleast > kmax) {
        // at least one digit
        kleast = kmax;
    }

    return {kleast, kmax};
}


bool mdn::Tools::toVecDigits(
    long double value,
    int base,
    VecDigit& digits,
    int& offset,
    std::pair<int, int>& kBand
) {
    // if (base < 2 || base > 32) throw std::invalid_argument("base must be in [2,32]");

    bool negative = std::signbit(value);
    int sign = 1;
    if (negative) {
        value *= -1;
        sign = -1;
    }
    value = std::fabs(value);
    if (!std::isfinite((double)value)) {
        Log_Warn("Non-finite value supplied");
        return false;
    }

    // Zero shortcut
    if (value == 0.0L) {
        digits   = {0};
        offset   = 0;
        negative = false;
        kBand.first = 0;
        kBand.second = 0;
        return true;
    }

    // Significance band in base 'base'
    // half-ULP absolute error bound
    const double delta = 0.5 * ulp(static_cast<double>(value));
    const long double lb = std::log(static_cast<long double>(base));

    const int kmax = (int)std::floor(std::log(static_cast<long double>(value)) / lb);
    int kleast = (int)std::floor(std::log(static_cast<long double>(delta)) / lb) + 1;
    if (kleast > kmax) {
        // keep at least one digit
        kleast = kmax;
    }

    unsigned long long Q;

    if ((base & (base - 1)) == 0) {
        // base is power of two
        int log2b = 0; unsigned b = base; while ((b >>= 1) != 0) ++log2b;
        const long double scaled_ld = std::scalbnl(value, -kleast * log2b);
        Q = static_cast<unsigned long long>(llround(scaled_ld));
    } else {
        const long double scale = std::pow(static_cast<long double>(base),
                                            static_cast<long double>(-kleast));
        const long double scaled_ld = value * scale;
        Q = static_cast<unsigned long long>(llround(scaled_ld));
    }

    // Strip least-significant base factors from Q so we don't emit LSD zeros.
    // This turns e.g. {0,0,1} with kleast=k into {1} with kleast=k+2 (offset unchanged).
    if (Q != 0ULL) {
        const unsigned B = static_cast<unsigned>(base);
        while (Q % B == 0ULL) {
            Q /= B;
            // we've shifted everything up by one exponent
            ++kleast;
        }
    }

    digits.clear();
    unsigned long long tmp = Q;
    while (tmp > 0ULL) {
        const int d = static_cast<int>(tmp % static_cast<unsigned>(base));
        digits.push_back(static_cast<Digit>(d)*sign);
        tmp /= static_cast<unsigned>(base);
    }

    // The last digit corresponds to exponent kleast + (digits.size()-1)
    offset = kleast + static_cast<int>(digits.size()) - 1;
    kBand.first = kleast;
    kBand.second = kmax;
    return true;
}
