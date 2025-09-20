#pragma once

#include <string>

#include "MdnQtInterface.hpp"

namespace mdn{
namespace gui{

enum class Operation {
    Add,
    Subtract,
    Multiply,
    Divide
};
static std::string OperationToString(const Operation& o) {
    switch (o) {
        case Operation::Add: {
            return "Add";
        }
        case Operation::Subtract: {
            return "Subtract";
        }
        case Operation::Multiply: {
            return "Multiply";
        }
        case Operation::Divide: {
            return "Divide";
        }
        default: {
            return "Unknown";
        }
    }
}
static std::string OperationToOpStr(const Operation& o) {
    switch (o) {
        case Operation::Add: {
            return "+";
        }
        case Operation::Subtract: {
            return "-";
        }
        case Operation::Multiply: {
            return "×";
        }
        case Operation::Divide: {
            return "÷";
        }
        default: {
            return "?";
        }
    }
}
static Operation StringToOperation(const std::string& s) {
    if (s == "Add" || s == "+") {
        return Operation::Add;
    } else if (s == "Subtract" || s == "-") {
        return Operation::Subtract;
    } else if (s == "Multiply" || s == "×") {
        return Operation::Multiply;
    }
    // s == "Divide" || s == "÷"
    return Operation::Divide;
}

// static std::string OperationToString(const Operation& o) {
static QString OperationToQString(const Operation& d) {
    return MdnQtInterface::toQString(OperationToString(d));
}
static QString OperationToOpQStr(const Operation& d) {
    return MdnQtInterface::toQString(OperationToOpStr(d));
}
static Operation QStringToOperation(const QString& s) {
    return StringToOperation(MdnQtInterface::fromQString(s));
}


inline std::ostream& operator<<(std::ostream& os, Operation e) {
    return os << OperationToString(e);
}

} // end namespace mdn
} // end namespace gui
