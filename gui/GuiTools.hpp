#pragma once

#include <QChar>

namespace mdn {
namespace gui {

class GuiTools {

public:

    // Utility to uppercase hex letters consistently
    static inline QChar toUpperAscii(QChar c)
    {
        if (c >= QChar('a') && c <= QChar('z')) {
            return QChar(c.unicode() - ('a' - 'A'));
        } else {
            return c;
        }
    }

};

} // end namespace gui
} // end namespace mdn
