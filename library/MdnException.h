// This header defines error-handling strategy for MDN operations
#pragma once

#include <stdexcept>
#include <string>
#include "Coord.h"

namespace mdn {

// Base class for all MDN-related runtime errors
class MdnException : public std::runtime_error {
public:
    explicit MdnException(const std::string& msg) : std::runtime_error(msg) {}
};

// Attempt to carry over from a zero digit
class InvalidCarryOver : public MdnException {
public:
    InvalidCarryOver(const Coord& xy)
        : MdnException("Coordinate " + xy.to_string() + ": invalid carry over at zero digit.") {}
};

// Attempt to assign out-of-range value to digit
class OutOfRange : public MdnException {
public:
    OutOfRange(const Coord& xy, int value, int base)
        : MdnException(
            "Coordinate " + xy.to_string() + ": out-of-range value " + std::to_string(value)
            + ", expecting ±" + std::to_string(base)
        ) {}
};

// Attempt to assign out-of-range value to digit
class ZeroEncountered : public MdnException {
public:
    ZeroEncountered(const Coord& xy)
        : MdnException(
            "Coordinate " + xy.to_string() + ": has recorded value of 0.  Sparse storage should " +
            "never contain a zero."
        ) {}
};

// MetaData is out of sync or invalid
class MetaDataInvalid : public MdnException {
public:
    MetaDataInvalid(const std::string& description)
        : MdnException("MDN MetaData invalid: " + description) {};
};

// If using a non-throwing path is preferable:
// enum class MdnStatus {
//     OK,
//     InvalidCarry,
//     OutOfBounds,
//     InvalidDigitRange
// };

} // namespace mdn
