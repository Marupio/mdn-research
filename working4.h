# MDN Clipboard v0 — Schema + Copy/Paste Code (Qt)

This file consolidates the **clipboard schema**, example payloads, and **reference implementations** for `copySelection`, `copyMdn`, and `pasteOnSelection(int index=-1)` (overwrite-only, bottom-left anchor). All control statements use braces per your style.

---

## MIME & JSON schema (v1)

**MIME**: `application/x-mdn-selection+json`

```json
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
```

Notes:

* `scope:"rect"` → payload from a grid selection; `rect` is the exact copied selection.
* `scope:"mdn"` → payload represents the entire MDN bounded by `bounds()` at copy-time; `rect` equals those bounds.
* `origin.rect` serves as a *default anchor* for tab-context paste; if missing, fall back to `(0,0)`.
* TSV is canonical for v0. External paste (e.g., Excel) uses the TSV fallback.

---

## Example payloads

### A) Selection (scope:"rect")

```json
{
  "type": "mdn-selection",
  "version": 1,
  "scope": "rect",
  "origin": {"mdn": "Foo", "rect": {"x0": 0, "y0": 0, "x1": 3, "y1": 2}},
  "rect": {"x0": 0, "y0": 0, "x1": 3, "y1": 2},
  "order": {"rows": "y_asc", "cols": "x_asc"},
  "grid_tsv": "1\t2\t3\t4\n5\t0\t0\t9\n0\t0\t0\t0\n"
}
```

### B) Whole MDN (scope:"mdn")

```json
{
  "type": "mdn-selection",
  "version": 1,
  "scope": "mdn",
  "origin": {"mdn": "Bar", "rect": {"x0": -2, "y0": -1, "x1": 4, "y1": 3}},
  "rect": {"x0": -2, "y0": -1, "x1": 4, "y1": 3},
  "order": {"rows": "y_asc", "cols": "x_asc"},
  "grid_tsv": "..."
}
```

---

## Paste rules (reference)

* **Overwrite-only**: source zeros clear target (via `setRowRange`).
* **Anchor is bottom-left** (x→right, y→up). Selection min corner is bottom-left.
* **Grid context (selection rect is valid)**:

  1. If selection is **1×1** → paste with that cell as bottom-left anchor.
  2. Else if selection **size matches** payload **exactly** → overwrite selection.
  3. Else → *error*: size mismatch; show user-facing message and abort.
* **Tab context (selection rect invalid)**:

  * `scope:"rect"` → anchor at `origin.rect` bottom-left if present, else `(0,0)`.
  * `scope:"mdn"` → clear target MDN and paste at `origin.rect` bottom-left if present, else `(0,0)`.

---

## C++ helpers (Project.cpp)

> Replace `getMdnByIndex(int)` with your actual accessor. Uses Qt types and your `mdn` APIs.

```cpp
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
```

### Copy helpers

```cpp
// Encodes a rectangular slice from src as mdn-selection JSON + TSV fallbacks.
static void encodeRectToClipboard(const Mdn2d& src, const Rect& r, const QString& scope,
                                  const QString& originMdnName) {
    Rect rr = r; rr.fixOrdering();
    const int x0 = rr.left();
    const int x1 = rr.right();
    const int y0 = rr.bottom();
    const int y1 = rr.top();

    QString tsv;
    tsv.reserve(rr.width() * rr.height() * 2);

    std::vector<Digit> row;
    row.reserve(rr.width());

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        const bool ok = src.getRowRange(y, x0, x1, row);
        if (!ok) {
            // Internal error safeguard: still build a zero row of correct width.
            row.assign(size_t(rr.width()), Digit(0));
        }
        tsv += joinDigitsTSV(row);
        tsv += '\n';
    }

    QJsonObject root;
    root["type"] = "mdn-selection";
    root["version"] = 1;
    root["scope"] = scope;

    QJsonObject origin;
    origin["mdn"] = originMdnName;
    origin["rect"] = rectToJson(rr);
    root["origin"] = origin;

    root["rect"] = rectToJson(rr); // for scope=="mdn", this equals bounds()

    QJsonObject order;
    order["rows"] = "y_asc";
    order["cols"] = "x_asc";
    root["order"] = order;

    root["grid_tsv"] = tsv;

    auto* mime = new QMimeData();
    mime->setData("application/x-mdn-selection+json",
                  QJsonDocument(root).toJson(QJsonDocument::Compact));
    mime->setData("text/tab-separated-values", tsv.toUtf8());
    mime->setText(tsv);

    QGuiApplication::clipboard()->setMimeData(mime); // takes ownership
}
```

### Public copy APIs

```cpp
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
```

### Decoding from clipboard

```cpp
static DecodedPaste decodeClipboard() {
    DecodedPaste out;

    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if (!mime) {
        return out;
    }

    QString tsv;

    if (mime->hasFormat("application/x-mdn-selection+json")) {
        const auto bytes = mime->data("application/x-mdn-selection+json");
        QJsonParseError err{};
        const auto doc = QJsonDocument::fromJson(bytes, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            const auto obj = doc.object();
            if (obj.value("type").toString() == QLatin1String("mdn-selection")) {
                out.scope = obj.value("scope").toString();
                if (obj.contains("rect") && obj.value("rect").isObject()) {
                    const auto ro = obj.value("rect").toObject();
                    out.srcRect = Rect(
                        ro.value("x0").toInt(), ro.value("y0").toInt(),
                        ro.value("x1").toInt(), ro.value("y1").toInt(),
                        /*fixOrdering*/true
                    );
                }
                tsv = obj.value("grid_tsv").toString();
            }
        }
    }

    if (tsv.isEmpty() && mime->hasFormat("text/tab-separated-values")) {
        tsv = QString::fromUtf8(mime->data("text/tab-separated-values"));
    }
    if (tsv.isEmpty() && mime->hasText()) {
        tsv = mime->text();
    }

    if (tsv.isEmpty()) {
        return out;
    }

    const QStringList lines = tsv.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return out;
    }

    out.rows.reserve(lines.size());
    for (const QString& line : lines) {
        const QStringList parts = line.split('\t', Qt::KeepEmptyParts);
        std::vector<Digit> row;
        row.reserve(parts.size());
        for (const QString& s : parts) {
            bool ok = false;
            const int v = s.toInt(&ok);
            row.push_back(static_cast<Digit>(ok ? v : 0));
        }
        out.rows.push_back(std::move(row));
    }

    return out;
}
```

### Paste implementation (v0 rules)

```cpp
// Returns false on user-facing size mismatch or invalid input; true on success.
bool Project::pasteOnSelection(int index /* = -1 */) {
    const mdn::Selection& sel = selection();

    Mdn2d* dst = nullptr;
    if (index >= 0) {
        dst = getMdnByIndex(index); // TODO: replace with your accessor
    } else {
        dst = sel.get();
    }
    if (dst == nullptr) {
        return false;
    }

    DecodedPaste p = decodeClipboard();
    if (!p.valid()) {
        return false;
    }

    const int W = p.width();
    const int H = p.height();

    const bool haveSel = (index < 0) && sel.rect.isValid();

    int ax = 0;
    int ay = 0;

    if (haveSel) {
        const Rect r = [&]() { Rect t = sel.rect; t.fixOrdering(); return t; }();
        const int SW = r.width();
        const int SH = r.height();
        if (SW == 1 && SH == 1) {
            ax = r.left();
            ay = r.bottom();
        } else if (SW == W && SH == H) {
            ax = r.left();
            ay = r.bottom();
        } else {
            // Size mismatch in grid context
            QMessageBox::warning(
                nullptr,
                QStringLiteral("Paste size mismatch"),
                QString("Selection %1x%2 vs data %3x%4").arg(SW).arg(SH).arg(W).arg(H)
            );
            return false;
        }
    } else {
        // Tab context (no valid selection rect)
        if (p.scope == QLatin1String("rect")) {
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
        } else if (p.scope == QLatin1String("mdn")) {
            dst->clear();
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
        } else {
            // Unknown scope: conservative default
            ax = 0;
            ay = 0;
        }
    }

    // Overwrite rows (zeros in payload clear cells)
    for (int r = 0; r < H; ++r) {
        dst->setRowRange(ay + r, ax, p.rows[size_t(r)]);
    }

    return true;
}
```

---

## Notes

* Coordinates: **x rightwards**, **y upwards**. Bottom-left = min corner.
* All rects are **inclusive** in both axes.
* `getRowRange(y, x0, x1, out)` and `setRowRange(y, x0, row)` are the fast paths.
* For now, we **don’t** warn on destructive `scope:"mdn"` pastes; add a prompt later.
* If your `Rect` header currently exposes `Rect::Invalid()` (capital **I**), simply change `Rect::invalid()` calls to match. Either casing is fine as long as you’re consistent project-wide.
