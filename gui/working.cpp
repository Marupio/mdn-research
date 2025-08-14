// Project.cpp — clipboard copy/paste v0 (overwrite), bottom-left anchor
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

using mdn::Digit;
using mdn::Mdn2d;
using mdn::Rect;
using mdn::Coord;

namespace { // local utilities

struct DecodedPaste {
    QString scope;              // "rect" | "mdn" | ""
    Rect    srcRect = Rect::invalid();
    std::vector<std::vector<Digit>> rows; // rows[r][c]
    bool valid()  const { return !rows.empty() && !rows.front().empty(); }
    int  width()  const { return valid() ? int(rows.front().size()) : 0; }
    int  height() const { return valid() ? int(rows.size())         : 0; }
};

static QString joinDigitsTSV(const std::vector<Digit>& v) {
    QStringList parts;
    parts.reserve(int(v.size()));
    for (Digit d : v) {
        parts << QString::number(int(d));
    }
    return parts.join('\t');
}

static QString rectToJsonStr(const Rect& r) {
    return QString("{\"x0\": %1, \"y0\": %2, \"x1\": %3, \"y1\": %4}")
        .arg(r.left()).arg(r.bottom()).arg(r.right()).arg(r.top());
}

static QJsonObject rectToJson(const Rect& r) {
    QJsonObject o;
    o["x0"] = r.left();
    o["y0"] = r.bottom();
    o["x1"] = r.right();
    o["y1"] = r.top();
    return o;
}

} // anonymous namespace
