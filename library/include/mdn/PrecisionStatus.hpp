#pragma once

// NAMED_ENUM
// Text flag to indicate this is one of the classes that needs to be replaced with NamedEnum

// Precision status
//  Is this coordinate within the precision window; if not is it above or below?

#include <vector>
#include <string>

#include <mdn/Logger.hpp>

namespace mdn {

enum class PrecisionStatus {
    Below,
    Inside,
    Above
};

const std::vector<std::string> PrecisionStatusNames(
    {
        "Below",
        "Inside",
        "Above"
    }
);

inline std::string PrecisionStatusToName(PrecisionStatus precisionStatus) {
    int fi = int(precisionStatus);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= PrecisionStatusNames.size())
    //         Logger::instance().error("PrecisionStatus out of range: " + std::to_string(fi));
    // #endif
    return PrecisionStatusNames[fi];
}

inline PrecisionStatus NameToPrecisionStatus(const std::string& name) {
    for (int i = 0; i < PrecisionStatusNames.size(); ++i) {
        if (PrecisionStatusNames[i] == name) {
            return static_cast<PrecisionStatus>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid PrecisionStatus type: " << name << " expecting:" << std::endl;
    if (PrecisionStatusNames.size()) {
        oss << PrecisionStatusNames[0];
    }
    for (auto iter = PrecisionStatusNames.cbegin() + 1; iter != PrecisionStatusNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
