#pragma once

#include <QString>

#include "EnumOperation.hpp"
#include "MdnQtInterface.hpp"

namespace mdn {
namespace gui {

struct OperationPlan {
    Operation op;
    int indexA;
    int indexB;
    // indexDest: when overwriting an existing tab, contains tab index, -1 if creating new
    int indexDest;
    // newName: when writing to a new tab, this is the name to try
    QString newName;

    friend std::ostream& operator<<(std::ostream& os, const OperationPlan& p) {
        std::string destStr;
        if (p.indexDest < 0) {
            destStr = "ToNew(" + MdnQtInterface::fromQString(p.newName) + ")";
        } else {
            destStr = "Overwrite(" + std::to_string(p.indexDest) + ")";
        }
        os << "[" << p.indexA << OperationToOpStr(p.op) << p.indexB
            << "→" << destStr << "]";
        return os;
    }
};

} // end namespace gui
} // end namespace mdn
