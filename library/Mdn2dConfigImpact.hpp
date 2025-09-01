#pragma once

// NAMED_ENUM
// Text flag to indicate this is one of the classes that needs to be replaced with NamedEnum

#include <vector>
#include <string>

#include "Logger.hpp"

// Sign enumeration

namespace mdn {

enum class Mdn2dConfigImpact {
    Unknown,
    NoImpact,
    AllDigitsCleared,
    PossibleDigitLoss,
    PossiblePolymorphism,
    PossibleDigitLossAndPolymorphism
};

const std::vector<std::string> Mdn2dConfigImpactNames(
    {
        "Unknown",
        "NoImpact",
        "AllDigitsCleared",
        "PossibleDigitLoss",
        "PossiblePolymorphism",
        "PossibleDigitLossAndPolymorphism"
    }
);

const std::vector<std::string> Mdn2dConfigImpactDescriptions(
    {
        "Unknown",
        "No impact",
        "Total loss - all digits will be cleared",
        "Possible loss of some digits",
        "Possible change in appearance but not value (polymorphism)",
        "Possible loss of some digits and a possible change in appearance"
    }
);

inline std::string Mdn2dConfigImpactToName(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactNames[fi];
}

inline std::string Mdn2dConfigImpactToDescription(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactDescriptions[fi];
}

inline Mdn2dConfigImpact NameToMdn2dConfigImpact(const std::string& name) {
    for (int i = 0; i < Mdn2dConfigImpactNames.size(); ++i) {
        if (Mdn2dConfigImpactNames[i] == name) {
            return static_cast<Mdn2dConfigImpact>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Mdn2dConfigImpact type: " << name << " expecting:" << std::endl;
    if (Mdn2dConfigImpactNames.size()) {
        oss << Mdn2dConfigImpactNames[0];
    }
    for (auto iter = Mdn2dConfigImpactNames.cbegin() + 1; iter != Mdn2dConfigImpactNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
