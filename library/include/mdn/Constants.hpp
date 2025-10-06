#pragma once

#include <limits>

#include "Digit.hpp"

namespace mdn {

namespace constants {

constexpr int intMin = std::numeric_limits<int>::min();
constexpr int intMax = std::numeric_limits<int>::max();
constexpr Digit DigitMin = std::numeric_limits<Digit>::min();
constexpr Digit DigitMax = std::numeric_limits<Digit>::max();

constexpr float floatSmall = 1e-6;
constexpr double doubleSmall = 1e-12;

constexpr long double ldoubleGreat = std::numeric_limits<long double>::max();

} // end namspace constants

} // end namespace mdn
