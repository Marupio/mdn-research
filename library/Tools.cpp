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


std::string mdn::Tools::digitToAlpha(Digit value, char pos, char neg) {
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


std::string mdn::Tools::digitToAlpha(const std::vector<Digit>& array, char pos=' ', char neg='-') {
    std::string ret;
    for (Digit digit : array) {
        ret += digitToAlpha(digit, pos, neg);
    }
    return ret;
}


std::string mdn::Tools::digitToAlpha(int value, char pos=' ', char neg='-') {
    return digitToAlpha(Digit(value), pos, neg);
}


std::string mdn::Tools::digitToAlpha(long value, char pos=' ', char neg='-') {
    return digitToAlpha(Digit(value), pos, neg);
}


std::string mdn::Tools::digitToAlpha(long long value, char pos=' ', char neg='-') {
    return digitToAlpha(Digit(value), pos, neg);
}


std::string mdn::Tools::digitToAlpha(const std::vector<int>& array, char pos=' ', char neg='-') {
}


std::string mdn::Tools::digitToAlpha(const std::vector<long>& array, char pos=' ', char neg='-') {
}


std::string mdn::Tools::digitToAlpha(const std::vector<long long>& array, char pos=' ', char neg='-') {
}


