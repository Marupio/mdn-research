#pragma once
#include <limits>

namespace mdn {

namespace constants {

constexpr int intMin = std::numeric_limits<int>::min();
constexpr int intMax = std::numeric_limits<int>::max();
constexpr float floatSmall = 1e-6;
constexpr double doubleSmall = 1e-12;

} // end namspace constants

} // end namespace mdn
