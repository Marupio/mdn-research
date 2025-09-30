#pragma once

#include <string>
#include <QString>

#include "MdnQtInterface.hpp"

namespace mdn {
namespace gui {

enum class OperationPhase { Idle, PickB, PickDest, PickRem, ActiveDivision };

static std::string OperationPhaseToString(const OperationPhase& op) {
    switch (op) {
        case OperationPhase::Idle: {
            return "Idle";
        }
        case OperationPhase::PickB: {
            return "PickB";
        }
        case OperationPhase::PickDest: {
            return "PickDest";
        }
        case OperationPhase::PickRem: {
            return "PickRem";
        }
        default: { // case OperationPhase::ActiveDivision: {
            return "ActiveDivision";
        }
    }
}
static OperationPhase StringToOperationPhase(const std::string& s) {
    if (s == "PickB") {
        return OperationPhase::PickB;
    } else if (s == "PickDest") {
        return OperationPhase::PickDest;
    } else if (s == "PickRem") {
        return OperationPhase::PickRem;
    } else if (s == "ActiveDivision") {
        return OperationPhase::ActiveDivision;
    } else { // s == "Idle"
        return OperationPhase::Idle;
    }
}
static QString OperationPhaseToQString(const OperationPhase& d) {
    return MdnQtInterface::toQString(OperationPhaseToString(d));
}
static OperationPhase QStringToOperationPhase(const QString& s) {
    return StringToOperationPhase(MdnQtInterface::fromQString(s));
}

inline std::ostream& operator<<(std::ostream& os, OperationPhase e) {
    return os << OperationPhaseToString(e);
}


} // end namespace mdn
} // end namespace gui
