#pragma once

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

#include "../library/Digit.hpp"
#include "../library/Rect.hpp"
#include "../library/Mdn2d.hpp"


namespace mdn {
namespace gui {

class Clipboard {

public:

    // *** Public data types ***

    struct DecodedPaste {
        // "rect" | "mdn" | ""
        QString scope;

        Rect srcRect = Rect::GetInvalid();

        // rows[r][c]
        std::vector<std::vector<Digit>> rows;

        bool valid()  const { return !rows.empty() && !rows.front().empty(); }
        int  width()  const { return valid() ? int(rows.front().size()) : 0; }
        int  height() const { return valid() ? int(rows.size())         : 0; }
    };

    static DecodedPaste decodeClipboard();

    static void encodeRectToClipboard(
        const Mdn2d& src,
        const Rect& r,
        const QString& scope,
        const QString& originMdnName
    );

};

} // end namespace gui
} // end namespace mdn
