#include "Project.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

#include "../library/Logger.h"
#include "../library/MdnException.h"
#include "../library/Rect.h"
#include "Selection.h"
#include "MdnQtInterface.h"


int mdn::Project::m_untitledNumber = 0;

void mdn::Project::shiftMdnTabsRight(int start, int end, int shift) {
    int lastI = m_data.size() - 1;
    if (end > lastI || end < 0) {
        end = lastI;
    }
    if (start > end) {
        int tmp = start;
        start = end;
        end = tmp;
    }
    if (start > lastI) {
        // Already at the end
        return;
    }
    if (shift <= 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be zero or negative."
        );
        QMessageBox::critical(m_parent, "shiftMdnTabsRight", err.what());
        throw err;
    }

    for (int tabI = end; tabI >= start; --tabI) {
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI + shift;
        m_addressingIndexToName.erase(tabI);
        m_addressingIndexToName.insert({newIndex, curName});

        auto node = m_data.extract(tabI);
        node.key() = newIndex;
        m_data.insert(std::move(node));

        m_addressingNameToIndex[curName] = newIndex;
    }
}


void mdn::Project::shiftMdnTabsLeft(int start, int end, int shift) {
    int lastI = m_data.size() - 1;
    if (end > lastI || end < 0) {
        end = lastI;
    }
    if (start > end) {
        int tmp = start;
        start = end;
        end = tmp;
    }
    if (start > lastI) {
        // Already at the end
        return;
    }
    if (shift <= 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be zero or negative."
        );
        QMessageBox::critical(m_parent, "shiftMdnTabsRight", err.what());
        throw err;
    }
    for (int tabI = start + shift; tabI <= end; ++tabI) {
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI - shift;
        m_addressingIndexToName.erase(tabI);
        m_addressingIndexToName.insert({newIndex, curName});

        auto node = m_data.extract(tabI);
        node.key() = newIndex;
        m_data.insert(std::move(node));

        m_addressingNameToIndex[curName] = newIndex;
    }
}


mdn::Project::Project(MainWindow* parent, std::string name, int nStartMdn):
    m_parent(parent),
    m_name(name)
{
    if (m_name.empty()) {
        m_name = "untitled-" + std::to_string(m_untitledNumber++);
    }
    for (int i = 0; i < nStartMdn; ++i) {
        std::string nextName = Mdn2d::static_generateNextName();
        Mdn2d newMdn = Mdn2d::NewInstance(m_config, nextName);
        appendMdn(newMdn);
    }
}


std::string mdn::Project::requestMdnNameChange(
    const std::string& origName,
    const std::string& newName
) {
    if (origName == newName) {
        // Nothing to change
        return newName;
    }
    if (!mdnNameExists(origName)) {
        InvalidArgument err = InvalidArgument("No Mdn2d named '" + origName + "' exists.");
        QMessageBox::critical(m_parent, "Name Missing", err.what());
        throw err;
    }
    if (mdnNameExists(newName)) {
        std::ostringstream oss;
        oss << "Cannot rename '" << origName << "' as '" << newName
            << "'. Name already exists." << std::endl;
        QMessageBox::information(
            m_parent,
            "Project ",
            oss.str().c_str()
        );
        return origName;
    }
    int index = m_addressingNameToIndex[origName];
    m_addressingNameToIndex.erase(origName);
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName[index] = newName;

    // This is not needed here, caller does this
    // m_data[index].m_name = newName;
    return newName;
}


void mdn::Project::setConfig(Mdn2dConfig newConfig) {
    if (m_data.empty()) {
        m_config = newConfig;
        return;
    }
    // For now, assume config is the same across all Mdn2d's
    Mdn2d* first = firstMdn();
    AssertQ(first, "Failed to acquire firstMdn()");
    Mdn2dConfigImpact impact = first->assessConfigChange(newConfig);
    switch (impact) {
        case Mdn2dConfigImpact::NoImpact: {
            m_config = newConfig;
            return;
        }
        case Mdn2dConfigImpact::AllDigitsCleared: {
            QMessageBox::StandardButton reply = QMessageBox::question(
                m_parent,
                "Configuration Change",
                "Making this change will clear all digits from all Mdn tabs.\n\nAre you sure?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No // Default button
            );

            if (reply == QMessageBox::Yes) {
                for (auto& [index, mdn] : m_data) {
                    mdn.setConfig(newConfig);
                }
                m_config = newConfig;
                return;
            } else {
                // User cancelled
                return;
            }
            break;
        }
        case Mdn2dConfigImpact::PossibleDigitLoss:
        case Mdn2dConfigImpact::PossiblePolymorphism:
        case Mdn2dConfigImpact::PossibleDigitLossAndPolymorphism: {
            std::string description = Mdn2dConfigImpactToDescription(impact);
            std::string question(
                "Making this change will have this impact on all Mdn tabs:\n"
                    + description + "\n\n"
                    + "Are you sure?"
            );
            QMessageBox::StandardButton reply = QMessageBox::question(
                m_parent,
                "Configuration Change",
                question.c_str(),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No // Default button
            );

            if (reply == QMessageBox::Yes) {
                for (auto& [index, mdn] : m_data) {
                    mdn.setConfig(newConfig);
                }
                m_config = newConfig;
                return;
            } else {
                // User cancelled
                return;
            }
        }
        default: {
            // including Mdn2dConfigImpactNames::Unknown
            InvalidState err("Did not receive a valid Mdn2dConfigImpact response");
            Log_Error(err.what());
            throw err;
        }
    }
}


bool mdn::Project::contains(std::string name, bool warnOnFailure) const {
    int index = indexOfMdn(name);
    if (index < 0) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Mdn2d with name '" << name << "' does not exist.";
            QMessageBox::warning(m_parent, "contains", oss.str().c_str());
            return false;
        }
        return false;
    }
    const auto iter = m_data.find(index);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Mdn2d with name '" << name << "' exists in addressing but not in data.";
            QMessageBox::warning(m_parent, "contains", oss.str().c_str());
            return false;
        }
        return false;
    }
    const Mdn2d& num = iter->second;
    if (num.getName() != name) {
        return false;
    }
    return true;
}


bool mdn::Project::contains(int i, bool warnOnFailure) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "contains", oss.str().c_str());
            return false;
        }

        return false;
    }
    return true;
}


int mdn::Project::indexOfMdn(std::string name) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        return -1;
    }
    return iter->second;
}


std::string mdn::Project::nameOfMdn(int i) const {
    const auto iter = m_addressingIndexToName.find(i);
    if (iter == m_addressingIndexToName.cend()) {
        return "";
    }
    return iter->second;
}


const mdn::Mdn2d* mdn::Project::getMdn(int i, bool warnOnFailure) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "getMdn", oss.str().c_str());
        }
        return nullptr;
    }
    return &(iter->second);
}
mdn::Mdn2d* mdn::Project::getMdn(int i, bool warnOnFailure) {
    auto iter = m_data.find(i);
    if (iter == m_data.end()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "getMdn", oss.str().c_str());
        }
        return nullptr;
    }
    return &(iter->second);
}


const mdn::Mdn2d* mdn::Project::getMdn(std::string name, bool warnOnFailure) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid Mdn name (" << name << ")";
            QMessageBox::warning(m_parent, "getMdn", oss.str().c_str());
        }
        return nullptr;
    }
    int index = iter->second;
    return &(m_data.at(index));
}
mdn::Mdn2d* mdn::Project::getMdn(std::string name, bool warnOnFailure) {
    auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid Mdn name (" << name << ")";
            QMessageBox::warning(m_parent, "getMdn", oss.str().c_str());
        }
        return nullptr;
    }
    int index = iter->second;
    return &(m_data[index]);
}


const mdn::Mdn2d* mdn::Project::firstMdn(bool warnOnFailure) const {
    return getMdn(0, warnOnFailure);
}
mdn::Mdn2d* mdn::Project::firstMdn(bool warnOnFailure) {
    return getMdn(0, warnOnFailure);
}


const mdn::Mdn2d* mdn::Project::lastMdn(bool warnOnFailure) const {
    int lastI = m_data.size() - 1;
    return getMdn(lastI, warnOnFailure);
}
mdn::Mdn2d* mdn::Project::lastMdn(bool warnOnFailure) {
    int lastI = m_data.size() - 1;
    return getMdn(lastI, warnOnFailure);
}


void mdn::Project::insertMdn(Mdn2d& mdn, int index) {
    // For warning messages
    std::ostringstream oss;

    // Check parameters
    int warnings = 0;
    int maxNewIndex = m_data.size();
    if (index > maxNewIndex) {
        oss << "Attempting to insert Mdn at index " << index << ", expecting -1 .. " << maxNewIndex;
        oss << " Appending the Mdn to the end, instead.";
        warnings += 1;
        index = maxNewIndex;
    }
    if (index < 0) {
        index = maxNewIndex;
    }
    m_data.insert({index, mdn});
    const std::string& origName = mdn.getName();
    std::string newName;
    if (mdnNameExists(origName)) {
        newName = Mdn2d::static_generateCopyName(origName);
        if (warnings > 0) {
            oss << std::endl;
        }
        oss << "Mdn '" << origName << "' already exists. Renaming new Mdn to '" << newName << "'.";
        warnings += 1;
    }
    if (!oss.str().empty()) {
        QMessageBox::warning(m_parent, "insertMdn", oss.str().c_str());
    }

    // Shift addressing over
    if (index < maxNewIndex) {
        shiftMdnTabsRight(index);
    }
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName.insert({index, newName});
}


std::string mdn::Project::duplicateMdn(int index) {
    Mdn2d* src = getMdn(index, true);
    if (!src) {
        return "";
    }
    Mdn2d dup = Mdn2d::Duplicate(*src);
    insertMdn(dup, index + 1);
    return dup.getName();
}


std::string mdn::Project::duplicateMdn(const std::string& name) {
    Mdn2d* src = getMdn(name, true);
    if (!src) {
        return "";
    }
    int index = indexOfMdn(name);
    Mdn2d dup = Mdn2d::Duplicate(*src);
    insertMdn(dup, index + 1);
    return dup.getName();
}


bool mdn::Project::moveMdn(int fromIndex, int toIndex) {
    if (fromIndex == toIndex) {
        // Nothing to do
        return true;
    }
    if (!contains(fromIndex, true)) {
        return false;
    }
    // Extract the number and erase the addressing metadata
    auto node = m_data.extract(fromIndex);
    std::string name = node.mapped().getName();
    node.key() = toIndex;
    m_addressingIndexToName.erase(fromIndex);
    m_addressingNameToIndex.erase(name);
    if (fromIndex > toIndex) {
        // Shifting digits to the right
        shiftMdnTabsRight(fromIndex - 1, toIndex + 1);
    } else {
        // Shifting digits to the left
        shiftMdnTabsLeft(fromIndex + 1, toIndex - 1);
    }
    m_data.insert(std::move(node));
    m_addressingIndexToName.insert({toIndex, name});
    m_addressingNameToIndex.insert({name, toIndex});
    return true;
}


bool mdn::Project::moveMdn(const std::string& name, int toIndex) {
    int fromIndex = indexOfMdn(name);
    if (fromIndex < 0) {
        std::ostringstream oss;
        oss << "Mdn2d with name '" << name << "' does not exist.";
        QMessageBox::warning(m_parent, "moveMdn", oss.str().c_str());
        return false;
    }
    return moveMdn(fromIndex, toIndex);
}


bool mdn::Project::eraseMdn(int index) {
    if (!contains(index, true)) {
        return false;
    }
    std::string name = nameOfMdn(index);
    m_data.erase(index);
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    shiftMdnTabsLeft(index+1);
}


bool mdn::Project::eraseMdn(const std::string& name) {
    if (!contains(name, true)) {
        return false;
    }
    int index = indexOfMdn(name);
    AssertQ(index >= 0, "Failed to find the index of contained Mdn2d '" << name << "'.");
    m_data.erase(index);
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    shiftMdnTabsLeft(index+1);
}



using mdn::Digit;
using mdn::Mdn2d;
using mdn::Rect;
using mdn::Coord;

namespace { // local utilities

struct DecodedPaste {
    QString scope;              // "rect" | "mdn" | ""
    Rect    srcRect = Rect::GetInvalid();
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


static void encodeRectToClipboard(
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
        const bool ok = src.getRow(y, x0, x1, row);
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


void mdn::Project::copySelection() const {
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


void mdn::Project::copyMdn(int index) const {
    const Mdn2d* src = getMdn(index);
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


void mdn::Project::cutSelection() {
    copySelection();
    deleteSelection();
}


bool mdn::Project::pasteOnSelection(int index) {
    const mdn::Selection& sel = selection();

    Mdn2d* dst = nullptr;
    if (index >= 0) {
        dst = getMdn(index);
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


void mdn::Project::deleteSelection() {

}


