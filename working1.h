// PasteSelection.cpp — v0 overwrite paste, bottom-left anchor, style-compliant
// - Always overwrite (source zeros clear target)
// - Anchor is bottom-left (x->right, y->up)
// - Destination rules:
//   A) If selection is 1×1 -> paste starting at that cell.
//   B) Else if selection size matches payload -> paste over selection.
//   C) Else if no selection rect (tab-context paste):
//        - scope:"rect" -> anchor at payload's source rect bottom-left (or (0,0)).
//        - scope:"mdn"  -> clear target MDN, anchor at source rect bottom-left (or (0,0)).
//   D) Else -> size mismatch -> show QMessageBox and abort.

// Paste selection, always overwrite, anchor relative to bottom-left, source-->destination
//  Source scope (data on the clipboard)
//  A) "mdn"  - defines an entire Mdn for pasting
//  B) "rect" - defines a specific area on a specific Mdn
//
//  Destination scope (data currently selected, m_selection)
//  1. selection.hasMdnOnly    - target is the entire Mdn
//  2. selection.hasRectOnly   - invalid - need a Mdn for actual operation
//  3. selection.hasMdnAndRect - target is the specific area on the selected Mdn
//
//  A-1 - Mdn ->  Mdn       - replace entire target Mdn with source Mdn
//  A-2 - Mdn ->  Rect      - Not valid (app error)
//  A-3 - Mdn ->  Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
//  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
//  B-2 - Rect -> Rect      - Not valid (app error)
//  B-3 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check req'd
//      Size check, if target is 1x1, paste okay, use that as bottom left anchor, otherwise must
//      match exactly



#include <QClipboard>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QMessageBox>
#include <QStringList>

#include "Selection.h"
#include "Mdn2d.h"
#include "Rect.h"

namespace mdn {

namespace {

struct DecodedPaste {
    // "rect" | "mdn" | ""(unknown)
    std::string scope;

    mdn::Rect srcRect = mdn::Rect::Invalid();

    // rows[r][c]
    std::vector<std::vector<mdn::Digit>> rows;

    bool valid() const { return !rows.empty() && !rows.front().empty(); }
    int width()  const { return valid()? int(rows.front().size()) : 0; }
    int height() const { return valid()? int(rows.size())         : 0; }


    // "rect" | "mdn" | ""
    QString scope;

    // where it was copied from (optional)
    Rect    srcRect = Rect::Invalid();

    // rows[r][c]
    std::vector<std::vector<Digit>> rows;

    // True if Selection the decoded paste contains valid data
    bool isValid() const { return !rows.empty() && !rows.front().empty; }

    int width() const { return isValid() ? static_cast<int>(rows.front().size()) : 0; }

    int height() const { return isValid() ? static_cast<int>(rows.size()) : 0; }

};


bool parseTsv(const QString& tsv, std::vector<std::vector<Digit>>& out)
{
    out.clear();

    const QStringList lines = tsv.split(u'\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return false;
    }

    for (const QString& line : lines) {
        const QStringList parts = line.split(u'\t', Qt::KeepEmptyParts);
        std::vector<Digit> row;
        row.reserve(parts.size());

        for (const QString& s : parts) {
            bool ok = false;
            const int v = s.toInt(&ok);
            if (!ok) {
                return false; // strict numeric TSV for v0
            }
            row.push_back(static_cast<Digit>(v));
        }

        out.push_back(std::move(row));
    }

    const std::size_t w = out.front().size();
    for (const auto& r : out) {
        if (r.size() != w) {
            return false; // must be rectangular
        }
    }

    return true;
}

DecodedPaste decodeClipboard()
{
    DecodedPaste p;

    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if (!mime) {
        return p;
    }

    if (mime->hasFormat("application/x-mdn-selection+json")) {
        const QByteArray bytes = mime->data("application/x-mdn-selection+json");
        QJsonParseError err{};
        const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);

        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject obj = doc.object();
            if (obj.value("type").toString() == "mdn-selection") {
                // scope (optional -> default to "rect")
                p.scope = obj.value("scope").toString();
                if (p.scope.isEmpty()) {
                    p.scope = QStringLiteral("rect");
                }

                // source rect (optional; used for tab-context paste anchor)
                if (obj.contains("rect")) {
                    const QJsonObject r = obj.value("rect").toObject();
                    const int x0 = r.value("x0").toInt();
                    const int y0 = r.value("y0").toInt();
                    const int x1 = r.value("x1").toInt();
                    const int y1 = r.value("y1").toInt();
                    p.srcRect = Rect(x0, y0, x1, y1, true /*fixOrdering*/);
                }

                // grid
                const QString tsv = obj.value("grid_tsv").toString();
                if (!tsv.isEmpty() && parseTsv(tsv, p.rows)) {
                    return p;
                }
            }
        }
    }

    // TSV fallbacks
    QString tsv;
    if (mime->hasFormat("text/tab-separated-values")) {
        tsv = QString::fromUtf8(mime->data("text/tab-separated-values"));
    } else if (mime->hasText()) {
        tsv = mime->text();
    }

    if (!tsv.isEmpty() && parseTsv(tsv, p.rows)) {
        p.scope = QStringLiteral("rect");
        return p;
    }

    return DecodedPaste{};
}

void showSizeMismatchDialog(int selW, int selH, int dataW, int dataH)
{
    QMessageBox::warning(
        nullptr,
        QStringLiteral("Paste size mismatch"),
        QStringLiteral("Selection %1×%2 vs data %3×%4")
            .arg(selW)
            .arg(selH)
            .arg(dataW)
            .arg(dataH)
    );
}

} // namespace (anon)

// Public entry — overwrite-only paste, returns void (KISS v0)
void Project::pasteOnSelection()
{
    const Selection& sel = selection();
    Mdn2d* dst = sel.get();

    if (!dst) {
        return;
    }

    DecodedPaste p = decodeClipboard();
    if (!p.isValid()) {
        return;
    }

    const int W = p.width();
    const int H = p.height();

    // Decide anchor according to rules A–D
    const bool haveSel = sel.rect.isValid();

    int ax = 0;
    int ay = 0;

    if (haveSel) {
        const int SW = sel.rect.width();
        const int SH = sel.rect.height();

        if (SW == 1 && SH == 1) {
            ax = sel.rect.left();
            ay = sel.rect.bottom();
        } else if (SW == W && SH == H) {
            ax = sel.rect.left();
            ay = sel.rect.bottom();
        } else {
            showSizeMismatchDialog(SW, SH, W, H);
            return;
        }
    } else {
        if (p.scope == QLatin1String("rect")) {
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
        } else if (p.scope == QLatin1String("mdn")) {
            // Replace entire MDN
            dst->clear();
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
        } else {
            // Unknown scope -> conservative default
            ax = 0;
            ay = 0;
        }
    }

    // Apply: one row at a time (zeros clear)
    for (int r = 0; r < H; ++r) {
        dst->setRowRange(ay + r, ax, p.rows[static_cast<std::size_t>(r)]);
    }
}

} // namespace mdn
