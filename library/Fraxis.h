#pragma once

#include <vector>
#include <string>

// Fraxis - fraction axis
//  The digit axis along which the fractional part of a real number expands in an MDN

namespace mdn {

enum class Fraxis {
    Invalid,
    Unknown,
    X,
    Y
};

const std::vector<std::string> FraxisNames(
    {
        "Invalid",
        "Unknown",
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

} // end namespace mdn
