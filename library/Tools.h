#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "Digit.h"

namespace mdn {

class Tools {

public:
    static const std::vector<char> m_digToAlpha;
    static const std::string m_boxArt_h; // ─
    static const std::string m_boxArt_v; // │
    static const std::string m_boxArt_x; // ┼

    // Join a vector of anything with a string delimiter
    template <typename T>
    static std::string joinArray(const std::vector<T>& array, const std::string& delimiter) {
        if (array.empty()) return "";

        std::ostringstream oss;
        oss << array[0];
        for (size_t i = 1; i < array.size(); ++i) {
            oss << delimiter << array[i];
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template <typename T>
    static std::string joinArray(const std::vector<T>& array, char delimiter) {
        return joinArray(array, std::string(1, delimiter));
    }

    // Converts a value between -32 and 32 to -V,..,-C,-B,-A,-9,-8, .. ,-1,0,1,...,8,9,A,B,...,V
    static std::string digitToAlpha(
        Digit value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        int value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long long value, std::string pos=" ", std::string neg=m_boxArt_h
    );

    static std::string digitToAlpha(
        const std::vector<Digit>& array, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        const std::vector<int>& array, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        const std::vector<long>& array, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        const std::vector<long long>& array, std::string pos=" ", std::string neg=m_boxArt_h
    );
};

} // namespace mdn
