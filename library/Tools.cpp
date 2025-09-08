#include <filesystem>

#include "Tools.hpp"

const std::vector<char> mdn::Tools::m_digToAlpha(
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
            ret = prefix + m_digToAlpha[value];
        } else {
            // May have to break value into pieces, maximum two digits
            int intVal = static_cast<int>(value);
            static const int ten = 10;
            int tens = intVal%ten;
            int ones = intVal/ten;
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