#pragma once

#include <string>
#include <QString>

#include "MdnQtInterface.hpp"

namespace mdn {
namespace gui {

enum class OperationPhase { Idle, PickB, PickDest };

static std::string OperationPhaseToString(const OperationPhase& op) {
    switch (op) {
        case OperationPhase::Idle: {
            return "Idle";
        }
        case OperationPhase::PickB: {
            return "PickB";
        }
        default: { // case OperationPhase::PickDest: {
            return "PickDest";
        }
    }
}
static OperationPhase StringToOperationPhase(const std::string& s) {
    if (s == "PickB") {
        return OperationPhase::PickB;
    } else if (s == "PickDest") {
        return OperationPhase::PickDest;
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
