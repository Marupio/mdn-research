#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Sign enumeration

namespace mdn {

enum class SignConvention {
    Invalid,
    Default,
    Positive,
    Negative
};

const std::vector<std::string> SignConventionNames(
    {
        "Invalid",
        "Default",  // Or, do not change
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
}


std::ostream& operator<<(std::ostream& os, const SignConvention& s) {
    os << SignConventionToName(s);
}

std::istream& operator>>(std::istream& is, SignConvention& s) {

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
