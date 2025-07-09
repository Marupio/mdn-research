#include <filesystem>

#include "Tools.h"

const std::vector<char> mdn::Tools::m_digToAlpha(
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V',
    }
);

const std::string mdn::Tools::m_boxArt_h = u8"\u2500"; // ─
const std::string mdn::Tools::m_boxArt_v = u8"\u2502"; // │
const std::string mdn::Tools::m_boxArt_x = u8"\u253C"; // ┼


std::string mdn::Tools::digitToAlpha(Digit value, std::string pos, std::string neg) {
    std::string ret;
    if (value < 0) {
        ret += neg;
        value = -value;
    } else {
        ret += pos;
    }
    if (value >= 0 && value <= 9) {
        ret += char('0' + value);
    } else if (value >= 10 && value <= 31) {
        ret += char('A' + (value - 10));
    } else {
    #ifdef MDN_DEBUG
        throw std::out_of_range(
            "digitToAlpha: value out of range (0..31): " + std::to_string(value)
        );
    #else
        ret = "??"; // fallback for release builds
    #endif
    }
    return ret;
}


std::string mdn::Tools::digitToAlpha(int value, std::string pos, std::string neg) {
    return digitToAlpha(Digit(value), pos, neg);
}


std::string mdn::Tools::digitToAlpha(long value, std::string pos, std::string neg) {
    return digitToAlpha(Digit(value), pos, neg);
}


std::string mdn::Tools::digitToAlpha(long long value, std::string pos, std::string neg) {
    return digitToAlpha(Digit(value), pos, neg);
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
