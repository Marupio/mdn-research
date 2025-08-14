{
  "type": "mdn-selection",
  "version": 1,
  "scope": "rect",
  "origin": {
    "mdn": "<name or id>",
    "rect": {"x0": -4, "y0": 0, "x1": 12, "y1": 8}
  },
  "rect": {
    "x0": -4, "y0": 0, "x1": 12, "y1": 8
  },
  "order": {"rows": "y_asc", "cols": "x_asc"},
  "grid_tsv": "<tab-separated rows>"
}

{
  "type": "mdn-selection",
  "version": 1,
  "scope": "rect",
  "origin": {"mdn": "Foo", "rect": {"x0": 0, "y0": 0, "x1": 3, "y1": 2}},
  "rect": {"x0": 0, "y0": 0, "x1": 3, "y1": 2},
  "order": {"rows": "y_asc", "cols": "x_asc"},
  "grid_tsv": "1\t2\t3\t4\n5\t0\t0\t9\n0\t0\t0\t0\n"
}

{
  "type": "mdn-selection",
  "version": 1,
  "scope": "mdn",
  "origin": {"mdn": "Bar", "rect": {"x0": -2, "y0": -1, "x1": 4, "y1": 3}},
  "rect": {"x0": -2, "y0": -1, "x1": 4, "y1": 3},
  "order": {"rows": "y_asc", "cols": "x_asc"},
  "grid_tsv": "..."
}

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


// Copy current grid selection (scope:"rect"). Respects bottom-left inclusive rect.
void Project::copySelection() const {
    const mdn::Selection& sel = selection();
    const Mdn2d* src = sel.get();
    if (src == nullptr) {
        return;
    }
    if (!sel.rect.isValid()) {
        return;
    }

    Rect r = sel.rect; r.fixOrdering();
    encodeRectToClipboard(*src, r, QStringLiteral("rect"), QString::fromStdString(src->getName()));
}

// Copy whole MDN from a tab context (scope:"mdn").
void Project::copyMdn(int index) const {
    const Mdn2d* src = getMdnByIndex(index); // TODO: replace with your accessor
    if (src == nullptr) {
        return;
    }
    if (!src->hasBounds()) {
        // Nothing to copy (empty MDN)
        return;
    }

    const Rect b = src->bounds();
    encodeRectToClipboard(*src, b, QStringLiteral("mdn"), QString::fromStdString(src->getName()));
}
