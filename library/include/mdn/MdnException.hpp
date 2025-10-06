// This header defines error-handling strategy for MDN operations
#pragma once

#include <stdexcept>
#include <string>
#include <mdn/Coord.hpp>

namespace mdn {

// Base class for all MDN-related runtime errors
class MDN_API MdnException : public std::runtime_error {
public:
    explicit MdnException(const std::string& msg) : std::runtime_error(msg) {}
};

// Attempt to carry over from a zero digit
class MDN_API FailedAssertion : public MdnException {
public:
    FailedAssertion(const std::string& description)
        : MdnException(description) {}
};

// Attempt to carry over from a zero digit
class MDN_API InvalidCarryOver : public MdnException {
public:
    InvalidCarryOver(const Coord& xy)
        : MdnException("Coordinate " + xy.to_string() + ": invalid carry over at zero digit.") {}
};

// Attempt to assign out-of-range value to digit
class MDN_API OutOfRange : public MdnException {
public:
    OutOfRange(const Coord& xy, int value, int base)
        : MdnException(
            "Coordinate " + xy.to_string() + ": out-of-range value " + std::to_string(value)
            + ", expecting ±" + std::to_string(base-1)
        ) {}
};

// Attempt to assign out-of-range value to digit
class MDN_API ZeroEncountered : public MdnException {
public:
    ZeroEncountered(const Coord& xy)
        : MdnException(
            "Coordinate " + xy.to_string() + ": has recorded value of 0.  Sparse storage should " +
            "never contain a zero."
        ) {}
};

// MetaData is out of sync or invalid
class MDN_API MetaDataInvalid : public MdnException {
public:
    MetaDataInvalid(const std::string& description)
        : MdnException("MDN MetaData invalid: " + description) {};
};

// Argument cannot be self
class MDN_API IllegalSelfReference : public MdnException {
public:
    IllegalSelfReference(const std::string& description)
        : MdnException("Supplied argument must not be the exact same object: " + description) {};
};

// Bases must match
class MDN_API BaseMismatch : public MdnException {
public:
    BaseMismatch(int baseA, int baseB)
        : MdnException(
            "Bases of interacting Mdn2d objects must match: (" + std::to_string(baseA) +
            " != " + std::to_string(baseB) + ")"
        ) {};
};

// This operation breaks the governing laws of MDNs
class MDN_API IllegalOperation : public MdnException {
public:
    IllegalOperation(const std::string& description)
        : MdnException("Illegal operation: " + description) {};
};

// The objects or data have encountered an invalid state
class MDN_API InvalidState: public MdnException {
public:
    InvalidState(const std::string& description)
        : MdnException("Invalid state detected: " + description) {};
};

// Division by zero is about to occur
class MDN_API DivideByZero : public MdnException {
public:
    DivideByZero()
        : MdnException("Division by zero") {};
};

// Argument is not valid
class MDN_API InvalidArgument : public MdnException {
public:
    InvalidArgument(const std::string& description)
        : MdnException("Invalid argument detected: " + description) {};
};

// Read error
class MDN_API ReadError : public MdnException {
public:
    ReadError(const std::string& description)
        : MdnException("Read error: " + description) {};
};

// Cannot perform the operation for the given described reason
class MDN_API InvalidOperation : public MdnException {
public:
    InvalidOperation(const std::string& description)
        : MdnException("Invalid operation detected: " + description) {};
};


} // namespace mdn
