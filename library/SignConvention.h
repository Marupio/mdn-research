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

inline SignConvention NameToSignConvention(const std::string& name) {
    for (int i = 0; i < SignConventionNames.size(); ++i) {
        if (SignConventionNames[i] == name) {
            return static_cast<SignConvention>(i);
        }
    }
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


} // end namespace mdn
