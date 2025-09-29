#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Constants.hpp"
#include "Digit.hpp"
#include "GlobalConfig.hpp"

namespace mdn {

class MDN_API Tools {

public:

    // Lower case alphanumeric
    static const std::vector<char> m_digToAlphaLower;

    // Upper case alphanumeric
    static const std::vector<char> m_digToAlphaUpper;

    // Display case - choose best between upper / lower (use lower unless ambiguous)
    static const std::vector<char> m_digToAlphaDisplay;

    static const std::string BoxArtStr_h; // ─
    static const std::string BoxArtStr_v; // │
    static const std::string BoxArtStr_x; // ┼
    static const char32_t BoxArtChar_h; // ─
    static const char32_t BoxArtChar_v; // │
    static const char32_t BoxArtChar_x; // ┼

    template <class T>
    struct is_byte_like
    : std::bool_constant<
            std::is_integral_v<std::remove_cv_t<T>> &&
            sizeof(std::remove_cv_t<T>) == 1> {};
    template <class T>
    inline static constexpr bool is_byte_like_v = is_byte_like<T>::value;


    // Convert a vector of anything to a string delimiter
    template<typename T>
    static std::string vectorToString(
        const std::vector<T>& array, const std::string& delimiter, bool reverse
    ) {
        if (array.empty()) {
            return "";
        }

        std::ostringstream oss;
        if (reverse) {
            size_t lastI = array.size()-1;
            if constexpr (is_byte_like_v<T>)
                oss << static_cast<int>(array[lastI]);
            else
                oss << array[lastI];
            for (size_t i = lastI-1; i >= 0; --i) {
                if constexpr (is_byte_like_v<T>)
                    oss << delimiter << static_cast<int>(array[i]);
                else
                    oss << delimiter << array[i];
            }
        } else {
            if constexpr (is_byte_like_v<T>)
                oss << static_cast<int>(array[0]);
            else
                oss << array[0];
            for (size_t i = 1; i < array.size(); ++i) {
                if constexpr (is_byte_like_v<T>)
                    oss << delimiter <<static_cast<int>(array[i]);
                else
                    oss << delimiter << array[i];
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string vectorToString(const std::vector<T>& array, char delimiter, bool reverse) {
        return vectorToString(array, std::string(1, delimiter), reverse);
    }

    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, const std::string& delimiter) {
        std::ostringstream oss;
        oss << pair.first << delimiter << pair.second;
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, char delimiter) {
        return pairToString(pair, std::string(1, delimiter));
    }

    // Convert a set of anything to a string delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, const std::string& delimiter) {
        if (set.empty()) return "";

        std::ostringstream oss;
        bool first = true;
        for (T elem : set) {
            if (first) {
                first = false;
                oss << elem;
            } else {
                oss << delimiter << elem;
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, char delimiter) {
        return setToString(set, std::string(1, delimiter));
    }

    // Convert a set of anything to a string delimiter
    template<typename S, typename T>
    static std::string mapToString(
        const std::unordered_map<S, T>& map,
        const std::string& delimiter
    ) {
        if (map.empty()) return "";

        std::ostringstream oss;
        bool first = true;
        for (auto i : map) {
            if (first) {
                first = false;
                oss << "(" << i.first << " : " << i.second << ")";
            } else {
                oss << delimiter << "(" << i.first << " : " << i.second << ")";
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename S, typename T>
    static std::string mapToString(const std::unordered_map<S, T>& map, char delimiter) {
        return mapToString(map, std::string(1, delimiter));
    }

    // Converts a value between -32 and 32 to -V,..,-C,-B,-A,-9,-8, .. ,-1,0,1,...,8,9,A,B,...,V
    static std::string digitToAlpha(
        Digit value,
        bool alphaNumerics=true,
        std::string pos=" ",
        std::string neg=BoxArtStr_h,
        int padSpacesToWidth=0
    );
    static std::string digitToAlpha(
        int value,
        bool alphaNumerics=true,
        std::string pos=" ",
        std::string neg=BoxArtStr_h,
        int padSpacesToWidth=0
    );
    static std::string digitToAlpha(
        long value,
        bool alphaNumerics=true,
        std::string pos=" ",
        std::string neg=BoxArtStr_h,
        int padSpacesToWidth=0
    );
    static std::string digitToAlpha(
        long long value,
        bool alphaNumerics=true,
        std::string pos=" ",
        std::string neg=BoxArtStr_h,
        int padSpacesToWidth=0
    );

    // Convert a std::vector<Digit> to a string
    static std::string digitArrayToString(
        const std::vector<Digit>& array,
        char delim=',',
        char open='(',
        char close=')'
    );

    // Convert the given (uppercase) alphanumeric character, return int value
    // unsafe: assumes c is '0' .. '9' or 'A' .. 'Z'
    static int unsafe_alphaToDigit(char c);


    // Ensure div is not too close to zero
    static void stabilise(float& div);
    static void stabilise(double& div);

    // Get filename from full path
    static std::string removePath(const char* fullpath);

    // Decompose given string into a string and an integer, return int has -1 if none
    static std::pair<std::string, int> strInt(const std::string& inStr);

    // Convert string to a valid real number with arbitrary number of digits
    // intPart and fracPart break the digit expression into integers of 12 digits at a time:
    //       intPart=(999'999'999'999, 999'999'999'999, 999'999'999'999, ...)
    //      fracPart=(999'999'999'999, 999'999'999'999, 999'999'999'999, ...)
    // for example, pi might be expressed as:
    //       intPart=(3)
    //      fracPart=(141592653589, 793238462643, 383279502884, 197169399375, 105820974944, ...)
    // On failure returns a string describing the failure, on success returns an empty string
    static std::string stringToArbitraryPrecisionReal(
        bool& negative,
        std::vector<long>& intPart,
        std::vector<long>& fracPart,
        int base,
        const std::string& input
    );

    // Helper function to test for valid digits in a string
    static constexpr bool validUppercaseDigit(const char digit, int base) noexcept;

    // Checks if the supplied string would be a valid number in the given base, with
    //  * an optional leading negative '-' character,
    //  * an optional radix (e.g. decimal) character at any position in the string
    //  * all other characters being valid alphanumeric digits for the given base (2 .. 32),
    //  for example:
    //      * base 2  : 0..1
    //      * base 10 : 0..9
    //      * base 16 : 0..9, A-F (upper or lower case)
    //      * base 32 : 0..9, A-V
    // Returns:
    //      * an empty string if the string is valid
    //      * a non-empty string detailing the parsing problems if the string is not valid
    //  Outputs:
    //      * negative = true only if '-' appears at the start of the string
    //      * radix has the position of the '.', or -1 if not present
    //      * hasIntPart and hasFracPart are true only when those components are non-zero
    static std::string verifyStringAsReal(
        const std::string& strIn, int base,
        bool& negative, std::string& intPart, std::string& fracPart, int& radix
    );

    // Unit in the last place
    static double ulp(double x);

    // Get the significance band for x in the given base (no checks on base, assumes 2..32)
    //  returns {kMin, kMax}, which are the lowest and highest exponents to keep, respectively,
    //  kMin and kMax are *inclusive* bounds.
    static std::pair<int, int> significanceBand(long double x, int base);

    // How many fractional digits are actually meaningful?
    static inline int maxFracDigits(long double x, int base) {
        std::pair<int, int> s = significanceBand(x, base);
        // Fractional digits live at exponents < 0. We keep from kMin up to -1.
        return std::max(0, -s.first);
    }

    static bool toVecDigits(
        long double value,
        int base,
        VecDigit& digits,
        int& offset,
        std::pair<int, int>& kRange
    );

};

} // namespace mdn
