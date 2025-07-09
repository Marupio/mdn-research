#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "Constants.h"
#include "Digit.h"
#include "GlobalConfig.h"

namespace mdn {

class MDN_API Tools {

public:
    static const std::vector<char> m_digToAlpha;
    static const std::string m_boxArt_h; // ─
    static const std::string m_boxArt_v; // │
    static const std::string m_boxArt_x; // ┼

    // Join a vector of anything with a string delimiter
    template <typename T>
    static std::string joinArray(
        const std::vector<T>& array, const std::string& delimiter, bool reverse
    ) {
        if (array.empty()) return "";

        std::ostringstream oss;
        if (reverse) {
            size_t lastI = array.size()-1;
            oss << array[lastI];
            for (size_t i = lastI-1; i >= 0; --i) {
                oss << delimiter << array[i];
            }
        } else {
            oss << array[0];
            for (size_t i = 1; i < array.size(); ++i) {
                oss << delimiter << array[i];
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template <typename T>
    static std::string joinArray(const std::vector<T>& array, char delimiter, bool reverse) {
        return joinArray(array, std::string(1, delimiter), reverse);
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

    // Ensure div is not too close to zero
    static void stabilise(float& div);
    static void stabilise(double& div);

};

} // namespace mdn
