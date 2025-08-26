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

    static const std::vector<char> m_digToAlpha;
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
                oss << delimiter << array[i];
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

    // Ensure div is not too close to zero
    static void stabilise(float& div);
    static void stabilise(double& div);

    // Get filename from full path
    static std::string removePath(const char* fullpath);

};

} // namespace mdn
