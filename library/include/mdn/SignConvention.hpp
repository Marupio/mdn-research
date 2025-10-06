#pragma once

// NAMED_ENUM
// Text flag to indicate this is one of the classes that needs to be replaced with NamedEnum

#include <vector>
#include <string>

#include "Logger.hpp"

// Sign enumeration

namespace mdn {

enum class SignConvention {
    Invalid,
    Neutral,
    Positive,
    Negative
};

const std::vector<std::string> SignConventionNames(
    {
        "Invalid",
        "Neutral",  // Or, do not change
        "Positive",
        "Negative"
    }
);

inline std::string SignConventionToName(SignConvention SignConvention) {
    int fi = int(SignConvention);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= SignConventionNames.size())
    //         Logger::instance().error("SignConvention out of range: " + std::to_string(fi));
    // #endif
    return SignConventionNames[fi];
}

inline SignConvention NameToSignConvention(const std::string& name, bool throwOnError=true) {
    for (int i = 0; i < SignConventionNames.size(); ++i) {
        if (SignConventionNames[i] == name) {
            return static_cast<SignConvention>(i);
        }
    }
    if (throwOnError) {
        std::ostringstream oss;
        oss << "Invalid SignConvention type: " << name << " expecting:" << std::endl;
        if (SignConventionNames.size()) {
            oss << SignConventionNames[0];
        }
        for (auto iter = SignConventionNames.cbegin() + 1; iter != SignConventionNames.cend(); ++iter) {
            oss << ", " << name;
        }
        throw std::invalid_argument(oss.str());
    }
    return SignConvention::Invalid;
}


inline std::ostream& operator<<(std::ostream& os, const SignConvention& s) {
    os << SignConventionToName(s);
    return os;
}

inline std::istream& operator>>(std::istream& is, SignConvention& s) {
    std::string word;
    is >> word;

    try {
        s = NameToSignConvention(word);
    }
    catch (const std::invalid_argument& e) {
        is.setstate(std::ios::failbit);
        return is;
    }
    return is;
}

} // end namespace mdn
