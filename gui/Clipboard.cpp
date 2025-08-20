#include "Clipboard.hpp"

#include "MdnQtInterface.hpp"

void mdn::Clipboard::encodeRectToClipboard(
    const Mdn2d& src,
    const Rect& r,
    const QString& scope,
    const QString& originMdnName
) {
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
        src.getRow(Coord(x0, y), rr.width(), row);
        tsv += MdnQtInterface::joinDigitsTSV(row);
        tsv += '\n';
    }

    QJsonObject root;
    root["type"] = "mdn-selection";
    root["version"] = 1;
    root["scope"] = scope;

    const Mdn2dConfig& srcConfig = src.config();
    QJsonObject configDict;
    configDict["base"] = srcConfig.base();
    // string format: (b:10, p:16, s:Positive, c:20, f:X)
    configDict["precision"] = srcConfig.precision();
    configDict["signConvention"] =
        MdnQtInterface::toQString(SignConventionToName(srcConfig.signConvention()));
    if (!srcConfig.maxCarryoverItersIsDefault()) {
        configDict["carryoverIters"] = srcConfig.maxCarryoverIters();
    }
    configDict["fraxis"] = MdnQtInterface::toQString(FraxisToName(srcConfig.fraxis()));

    QJsonObject origin;
    origin["mdn"] = originMdnName;
    origin["rect"] = MdnQtInterface::rectToJson(rr);
    origin["config"] = configDict;
    root["origin"] = origin;
    root["rect"] = MdnQtInterface::rectToJson(rr); // for scope=="mdn", this equals bounds()

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


mdn::Clipboard::DecodedPaste mdn::Clipboard::decodeClipboard() {
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
