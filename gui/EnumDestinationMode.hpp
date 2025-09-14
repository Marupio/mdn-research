#pragma once

#include <string>
#include <QString>

#include "MdnQtInterface.hpp"

namespace mdn{
namespace gui{

// *** Full Destination Enumeration ***

enum class DestinationMode {
    OverwriteA,
    OverwriteB,
    CreateNew,
    OverwriteIndex
};
static std::string DestinationModeToString(const DestinationMode& d) {
    switch (d) {
        case DestinationMode::OverwriteA: {
            return "OverwriteA";
        }
        case DestinationMode::OverwriteB: {
            return "OverwriteB";
        }
        case DestinationMode::CreateNew: {
            return "CreateNew";
        }
        case DestinationMode::OverwriteIndex: {
            return "OverwriteIndex";
        }
        default: {
            return "Unknown";
        }
    }
}
static DestinationMode StringToDestinationMode(const std::string& s) {
    if (s == "OverwriteA") {
        return DestinationMode::OverwriteA;
    } else if (s == "OverwriteB") {
        return DestinationMode::OverwriteB;
    } else if (s == "CreateNew") {
        return DestinationMode::CreateNew;
    } else { // s == "OverwriteIndex"
        return DestinationMode::OverwriteIndex;
    }
}
static QString DestinationModeToQString(const DestinationMode& d) {
    return MdnQtInterface::toQString(DestinationModeToString(d));
}
static DestinationMode QStringToDestinationMode(const QString& s) {
    return StringToDestinationMode(MdnQtInterface::fromQString(s));
}

inline std::ostream& operator<<(std::ostream& os, DestinationMode e) {
    return os << DestinationModeToString(e);
}


// *** Simple Destination Enumeration ***

enum class DestinationSimple {
    InPlace,
    ToNew
};
static std::string DestinationSimpleToString(const DestinationSimple& d) {
    switch (d) {
        case DestinationSimple::InPlace: {
            return "InPlace";
        }
        case DestinationSimple::ToNew: {
            return "ToNew";
        }
        default: {
            return "Unknown";
        }
    }
}
static DestinationSimple StringToDestinationSimple(const std::string& s) {
    if (s == "InPlace") {
        return DestinationSimple::InPlace;
    }
    // s == "ToNew"
    return DestinationSimple::ToNew;
}
static QString DestinationSimpleToQString(const DestinationSimple& d) {
    return MdnQtInterface::toQString(DestinationSimpleToString(d));
}
static DestinationSimple QStringToDestinationSimple(const QString& s) {
    return StringToDestinationSimple(MdnQtInterface::fromQString(s));
}

inline std::ostream& operator<<(std::ostream& os, DestinationSimple e) {
    return os << DestinationSimpleToString(e);
}

} // end namespace mdn
} // end namespace gui
