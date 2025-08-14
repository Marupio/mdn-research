// PasteSelection.cpp — overwrite paste with bottom-left anchor
// Assumptions:
//  - Selection is single-active-MDN (finite rect optional — may be invalid when pasting via tab
//      menu)
//  - Clipboard prefers JSON (application/x-mdn-selection+json) with fields:
//      type:"mdn-selection", version:1, rect:{x0,y0,x1,y1}, order, grid_tsv
//  - Fallbacks: text/tab-separated-values, text/plain
//  - Coordinates: x increases -> right, y increases -> up; bottom-left is the min corner
//  - Overwrite semantics: source zeros CLEAR destination cells
//
// Behavioural rules implemented below:
//  (A) If selection is a single cell -> anchor paste at that cell (use it as bottom-left).
//  (B) Else if selection dimensions match the TSV rectangle -> paste over that selection rect.
//  (C) Else if selection is invalid/empty (e.g., tab-level paste) -> anchor at source rect's
//      bottom-left (from JSON). If JSON lacks rect, fall back to (0,0).
//  (D) Else (non-1x1 selection and size mismatch) -> abort and report size mismatch.
//
// Notes:
//  - This file is self-contained; wire into Project::pasteOnSelection().
//  - For UI feedback on errors, replace TODOs with your signal/dialog of choice.

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

#include "Selection.h"
#include "Mdn2d.h"
#include "Rect.h"

namespace mdn {

struct DecodedPaste {
    std::vector<std::vector<Digit>> rows; // rows[r][c]
    Rect srcRect = Rect::Invalid();       // where it was copied from (bottom-left anchor)
    bool valid() const { return !rows.empty() && !rows.front().empty(); }
    int  width()  const { return valid()? int(rows.front().size()) : 0; }
    int  height() const { return valid()? int(rows.size())         : 0; }
};

static bool parseTsv(const QString& tsv, std::vector<std::vector<Digit>>& out) {
    out.clear();
    const auto lines = tsv.split(u'\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) return false;

    for (const QString& line : lines) {
        const auto parts = line.split(u'\t', Qt::KeepEmptyParts);
        std::vector<Digit> row; row.reserve(parts.size());
        for (const QString& s : parts) {
            bool ok=false; int v=s.toInt(&ok);
            if (!ok) return false; // strict numeric TSV for now
            row.push_back(static_cast<Digit>(v));
        }
        out.push_back(std::move(row));
    }
    const size_t w = out.front().size();
    for (auto& r : out) {
        if (r.size()!=w) {
            return false; // must be rectangular
        }
    }
    return true;
}

static DecodedPaste decodeClipboard() {
    DecodedPaste p{};
    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if (!mime) return p;

    // Prefer our JSON
    if (mime->hasFormat("application/x-mdn-selection+json")) {
        const auto bytes = mime->data("application/x-mdn-selection+json");
        QJsonParseError err{}; const auto doc = QJsonDocument::fromJson(bytes, &err);
        if (err.error==QJsonParseError::NoError && doc.isObject()) {
            const auto obj = doc.object();
            if (obj.value("type").toString()=="mdn-selection") {
                // rect is optional but very useful for anchoring when no selection present
                if (obj.contains("rect")) {
                    const auto r = obj.value("rect").toObject();
                    const int x0=r.value("x0").toInt();
                    const int y0=r.value("y0").toInt();
                    const int x1=r.value("x1").toInt();
                    const int y1=r.value("y1").toInt();
                    p.srcRect = Rect(x0,y0,x1,y1,true/*fixOrdering*/);
                }
                const QString tsv = obj.value("grid_tsv").toString();
                if (!tsv.isEmpty() && parseTsv(tsv, p.rows)) return p;
            }
        }
    }
    // TSV fallbacks
    QString tsv;
    if (mime->hasFormat("text/tab-separated-values"))
        tsv = QString::fromUtf8(mime->data("text/tab-separated-values"));
    else if (mime->hasText())
        tsv = mime->text();

    if (!tsv.isEmpty() && parseTsv(tsv, p.rows)) return p;
    return {};
}

// Returns false on error (e.g., size mismatch case D)
bool Project::pasteOnSelection() {
    // 0) Resolve destination MDN (we allow no selection-rect for tab-level paste)
    const Selection& sel = selection();
    Mdn2d* dst = sel.get();
    if (!dst) return false;

    // 1) Decode clipboard
    DecodedPaste payload = decodeClipboard();
    if (!payload.valid()) return false;
    const int W = payload.width();
    const int H = payload.height();

    // 2) Compute anchor based on selection & rules A–D
    const bool haveRect = sel.rect.isValid();
    int ax=0, ay=0; // anchor (bottom-left)

    if (!haveRect) {
        // (C) No selection: anchor at source rect bottom-left, or (0,0) if absent
        if (payload.srcRect.isValid()) { ax = payload.srcRect.left(); ay = payload.srcRect.bottom(); }
        else { ax = 0; ay = 0; }
    } else if (sel.rect.width()==1 && sel.rect.height()==1) {
        // (A) Single cell -> use that as bottom-left
        ax = sel.rect.left(); ay = sel.rect.bottom();
    } else if (sel.rect.width()==W && sel.rect.height()==H) {
        // (B) Size match -> paste over selection rect
        ax = sel.rect.left(); ay = sel.rect.bottom();
    } else {
        // (D) Mismatch -> abort and notify
        // TODO: emit signal / dialog: "Paste size mismatch: selection %dx%d vs data %dx%d"
        return false;
    }

    // 3) Apply: overwrite semantics (zero clears)
    for (int r = 0; r < H; ++r)
        dst->setRowRange(ay + r, ax, payload.rows[size_t(r)]);

    // TODO: push undo command; notify views of data change
    return true;
}

} // namespace mdn
