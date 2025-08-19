#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// A few general tools to convert templated Enum into its associated name, and visa-versa
// Not actually used yet.

namespace mdn {

template<class Enum>
std::string EnumToName(Enum e, const std::vector<std::string>& names) {
    int ei = int(e);
    return names[ei];
}

template<class Enum>
Enum NameToEnum(const std::string& name, std::vector<std::string>& names) {
    for (int i = 0; i < names.size(); ++i) {
        if (names[i] == name) {
            return static_cast<Enum>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Enum type: " << name << " expecting:" << std::endl;
    if (names.size()) {
        oss << names[0];
    }
    for (auto iter = names.cbegin() + 1; iter != names.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
