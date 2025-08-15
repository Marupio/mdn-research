#include <filesystem>

#include "Tools.h"

const std::vector<char> mdn::Tools::m_digToAlpha(
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'L', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v',
    }
);

const std::string mdn::Tools::m_boxArt_h = u8"\u2500"; // ─
const std::string mdn::Tools::m_boxArt_v = u8"\u2502"; // │
const std::string mdn::Tools::m_boxArt_x = u8"\u253C"; // ┼


std::string mdn::Tools::digitToAlpha(
    Digit value,
    bool alphaNumerics,
    std::string pos,
    std::string neg
) {
    std::string prefix;
    if (value < 0) {
        prefix += neg;
        value = -value;
    } else {
        prefix += pos;
    }
    if (value >= 0 && value <= 31) {
        if (alphaNumerics) {
            return prefix + m_digToAlpha[value];
        } else {
            int intVal = static_cast<int>(value);
            static const int ten = 10;
            int tens = intVal%ten;
            int ones = intVal/ten;
            if (tens == 0) {
                std::string ret = " " + prefix + std::to_string(ones);
                return ret;
            }
            std::string ret = prefix + std::to_string(tens) + std::to_string(ones);
            return ret;
        }
    } else {
    #ifdef MDN_DEBUG
        throw std::out_of_range(
            "digitToAlpha: value out of range (0..31): " + std::to_string(value)
        );
    #else
        return "??"; // fallback for release builds
    #endif
    }
}


std::string mdn::Tools::digitToAlpha(
    int value,
    bool alphaNumerics,
    std::string pos,
    std::string neg
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg);
}


std::string mdn::Tools::digitToAlpha(
    long value,
    bool alphaNumerics,
    std::string pos,
    std::string neg
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg);
}


std::string mdn::Tools::digitToAlpha(
    long long value,
    bool alphaNumerics,
    std::string pos,
    std::string neg
) {
    return digitToAlpha(Digit(value), alphaNumerics, pos, neg);
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
