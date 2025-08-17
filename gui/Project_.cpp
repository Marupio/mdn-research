#include "Project.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonObject>
#include <QMimeData>

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


void mdn::Project::copySelection() const {
    const Selection& sel = selection();
    if (sel.isEmpty()) {
        return;
    }

    const mdn::Mdn2d* src = sel.get();
    if (!src) {
        return;
    }

    Rect r = sel.rect;
    // r.fixOrdering();
    if (!r.isValid()) {
        return;
    }

    QString tsv;
    tsv.reserve(r.width() * r.height() * 2);

    std::vector<Digit> rowBuf; rowBuf.reserve(r.width());
    for (int y = r.bottom(); y <= r.top(); ++y) {
        rowBuf.clear();
        src->getRow(y, r.left(), r.right(), rowBuf);
        const std::string line = Tools::vectorToString(rowBuf, '\t', false);
        tsv += QString::fromStdString(line);
        tsv += u'\n';
    }

    // JSON payload (v1)
    QJsonObject root;
    root["type"] = "mdn-selection";
    root["version"] = 1;
    root["origin_name"] = QString::fromStdString(src->getName()); // optional

    QJsonObject jrect;
    jrect["x0"] = r.left();   jrect["y0"] = r.bottom();
    jrect["x1"] = r.right();  jrect["y1"] = r.top();
    root["rect"] = jrect;

    QJsonObject order;
    order["rows"] = "y_asc"; order["cols"] = "x_asc";
    root["order"] = order;

    root["grid_tsv"] = tsv;

    auto* mime = new QMimeData();
    mime->setData(
        "application/x-mdn-selection+json",
        QJsonDocument(root).toJson(QJsonDocument::Compact)
    );
    mime->setData("text/tab-separated-values", tsv.toUtf8());
    mime->setText(tsv);

    QGuiApplication::clipboard()->setMimeData(mime);
}


void mdn::Project::cutSelection() {
    copySelection();
    deleteSelection();
}


namespace { // anon

struct DecodedPaste {
    // "rect" | "mdn" | ""
    QString scope;

    // where it was copied from (optional)
    mdn::Rect srcRect = mdn::Rect::GetInvalid();

    // rows[r][c]
    std::vector<std::vector<mdn::Digit>> rows;

    // True if Selection the decoded paste contains valid data
    bool isValid() const { return !rows.empty() && !rows.front().empty(); }

    int width() const { return isValid() ? static_cast<int>(rows.front().size()) : 0; }

    int height() const { return isValid() ? static_cast<int>(rows.size()) : 0; }

};


bool parseTsv(const QString& tsv, std::vector<std::vector<mdn::Digit>>& out)
{
    out.clear();

    const QStringList lines = tsv.split(u'\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) {
        return false;
    }

    for (const QString& line : lines) {
        const QStringList parts = line.split(u'\t', Qt::KeepEmptyParts);
        std::vector<mdn::Digit> row;
        row.reserve(parts.size());

        for (const QString& s : parts) {
            bool ok = false;
            const int v = s.toInt(&ok);
            if (!ok) {
                // strict numeric TSV for v0
                return false;
            }
            row.push_back(static_cast<mdn::Digit>(v));
        }

        out.push_back(std::move(row));
    }

    const std::size_t w = out.front().size();
    for (const auto& r : out) {
        if (r.size() != w) {
            // must be rectangular
            return false;
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
        // Pasting Mdn-formatted data
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
                    p.srcRect = mdn::Rect(x0, y0, x1, y1, true /*fixOrdering*/);
                }

                // grid
                const QString tsv = obj.value("grid_tsv").toString();
                if (!tsv.isEmpty() && parseTsv(tsv, p.rows)) {
                    return p;
                }
            } // if (obj.value("type").toString() == "mdn-selection")
        } // if (err.error == QJsonParseError::NoError && doc.isObject())
    }

    // From here:
    //  * not mdn-formatted data
    //  * mdn-formatted data that is not type=="mdn-selection"
    //  * mdn-formatted data that is type=="mdn-selection" but could not get the data:
    //      o fromJson returned an error
    //      o fromJson returned an invalid object
    //      o fromJson worked but tsv was empty or parseTsv returned false
    // Attempt tsv fallback
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


void mdn::Project::pasteOnSelection(int index) {
    Mdn2d* targetMdn = nullptr;
    Rect targetRect = Rect::GetInvalid();

    if (index < 0) {
        const Selection& sel = selection();
        // AssertQ(sel.hasMdn(), "Internal error: selection has no valid Mdn");
        if (!sel.hasMdn()) {
            // Selection has no valid Mdn - probably no Mdn tabs present, fail mostly silently
            #ifdef MDN_DEBUG
                Log_N_WarnQ("Attempting to 'paste' when no Mdn tab is in the selection.");
            #endif
            return;
        }
        targetMdn = sel.get();
        if (sel.hasRect()) {
            targetRect = sel.rect;
        }
    } else {
        // Not based on m_selection - an index has been supplied
        targetMdn = getMdn(index, true);
        if (!targetMdn) {
            return;
        }
    }

    DecodedPaste p = decodeClipboard();
    if (!p.isValid()) {
        return;
    }
    const int w = p.width();
    const int h = p.height();

    // Determine bottom-left anchor
    int ax = 0;
    int ay = 0;

    if (targetRect.isValid()) {
        // A-2 or B-2
        const int rw = targetRect.width();
        const int rh = targetRect.height();

        if (rw == 1 && rh == 1) {
            ax = targetRect.left();
            ay = targetRect.bottom();
        } else if (rw == w && rh == h) {
            ax = targetRect.left();
            ay = targetRect.bottom();
        } else {
            showSizeMismatchDialog(rw, rh, w, h);
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
            targetMdn->clear();
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
    for (int r = 0; r < h; ++r) {
        targetMdn->setRow(ay + r, ax, p.rows[static_cast<std::size_t>(r)]);
    }
}


void mdn::Project::deleteSelection() {

}


