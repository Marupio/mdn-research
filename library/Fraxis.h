#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Fraxis - fraction axis
//  The digit axis along which the fractional part of a real number expands in an MDN

namespace mdn {

enum class Fraxis {
    Invalid,
    Default,
    X,
    Y
};

const std::vector<std::string> FraxisNames(
    {
        "Invalid",
        "Default",
        "X",
        "Y"
    }
);

std::string FraxisToName(Fraxis fraxis) {
    int fi = int(fraxis);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= FraxisNames.size())
    //         Logger::instance().error("Fraxis out of range: " + std::to_string(fi));
    // #endif
    return FraxisNames[fi];
}

Fraxis NameToFraxis(const std::string& name) {
    for (int i = 0; i < FraxisNames.size(); ++i) {
        if (FraxisNames[i] == name) {
            return static_cast<Fraxis>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Fraxis type: " << name << " expecting:" << std::endl;
    if (FraxisNames.size()) {
        oss << FraxisNames[0];
    }
    for (auto iter = FraxisNames.cbegin() + 1; iter != FraxisNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
