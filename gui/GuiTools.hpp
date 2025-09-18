#pragma once

#include <QChar>
#include <QColor>
#include <QTabBar>
#include <QTabWidget>

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

    template<typename T>
    static void binaryWrite(std::ostream& out, const T& v) {
        out.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }
    template<typename T>
    static void binaryRead(std::istream& in, T& v) {
        in.read(reinterpret_cast<char*>(&v), sizeof(T));
    }
    static void binaryWriteString(std::ostream& out, const std::string& s) {
        const uint32_t n = static_cast<uint32_t>(s.size());
        binaryWrite(out, n);
        out.write(s.data(), n);
    }
    static std::string binaryReadString(std::istream& in) {
        uint32_t n = 0; binaryRead(in, n);
        std::string s; s.resize(n);
        in.read(s.data(), n);
        return s;
    }

    static void setTabPeekHighlight(QTabWidget* tabs, int idx, bool on) {
        if (!tabs) return;
        auto* b = tabs->tabBar();
        if (!b || idx < 0 || idx >= b->count()) return;
        if (on) {
            b->setTabTextColor(idx, QColor(Qt::blue));
        } else {
            // default colour
            b->setTabTextColor(idx, QColor());
        }
    }

};

} // end namespace gui
} // end namespace mdn
