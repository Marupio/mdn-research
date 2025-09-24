// Online C++ compiler to run C++ program online
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>

std::string stringToArbitraryPrecisionReal(
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
                // Stop if this fractional state repeats (decimal expansion would be non-terminating)
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
                // when the fraction truly terminates in decimal (e.g., bases with factors 2 and/or 5).
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

int main() {
    std::string instr;
    bool keepGoing = true;
    while (keepGoing) {
        int base = -1;
        while (base < 0) {
            std::cout << "\nBase ? ";
            std::getline(std::cin, instr);
            if (instr == "quit" || instr == "exit") {
                return 0;
            }
            int tmp = std::atoi(instr.c_str());
            if (tmp >= 2 <= 32) {
                base = tmp;
            }
        }
        std::cout << "Number ? ";
        std::getline(std::cin, instr);
        if (instr == "quit" || instr == "exit") {
            return 0;
        }
        bool negative = false;
        std::vector<long> intPart;
        std::vector<long> fracPart;
        std::string message(
            stringToArbitraryPrecisionReal(
                negative,
                intPart,
                fracPart,
                base,
                instr
            )
        );
        if (message.empty()) {
            std::cout << "\nSuccess!\n" << std::endl;
            std::cout << "negative=" << negative << std::endl;
            for (long e : intPart) {
                std::cout << e << "'";
            }
            std::cout << ".";
            for (long e : fracPart) {
                std::cout << e << "'";
            }
            std::cout << "\n\nNext..." << std::endl;
        } else {
            std::cout << "\nFailure!\n" << std::endl;
            std::cout << "Message=[" << message
                << "]\n" << std::endl;
        }
    }
    return 0;
}