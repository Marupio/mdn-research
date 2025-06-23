// This header defines error-handling strategy for MDN operations
#pragma once

#include <stdexcept>
#include <string>

namespace mdn {

// Base class for all MDN-related runtime errors
class MdnException : public std::runtime_error {
public:
    explicit MdnException(const std::string& msg) : std::runtime_error(msg) {}
};

// Specific error: attempt to carry over from a zero digit
class InvalidCarryOver : public MdnException {
public:
    InvalidCarryOver(int x, int y)
        : MdnException("Invalid carry-over at empty coordinate (" + std::to_string(x) + ", " + std::to_string(y) + ")") {}
};

// Optionally, define other specific exceptions as needed

// If using a non-throwing path is preferable:
// enum class MdnStatus {
//     OK,
//     InvalidCarry,
//     OutOfBounds,
//     InvalidDigitRange
// };

} // namespace mdn
