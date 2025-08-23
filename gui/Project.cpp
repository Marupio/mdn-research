#include "Project.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

#include "../library/Logger.hpp"
#include "../library/MdnException.hpp"
#include "../library/Rect.hpp"
#include "../library/SignConvention.hpp"
#include "Clipboard.hpp"
#include "Selection.hpp"
#include "MainWindow.hpp"
#include "MdnQtInterface.hpp"


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
    if (nStartMdn == 0) {
        nStartMdn += 1;
    }
    for (int i = 0; i < nStartMdn; ++i) {
        std::string nextName = Mdn2d::static_generateNextName();
        Mdn2d newMdn = Mdn2d::NewInstance(m_config, nextName);
        appendMdn(newMdn);
    }
    setActiveMdn(0);
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


void mdn::Project::updateSelection() const {
    if (!m_parent) {
        return;
    }
    // Not yet implemented:
    // m_parent->updateSelection(m_activeMdn2d, m_activeSelection);
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
                for (auto& [index, mdnAndSel] : m_data) {
                    mdnAndSel.first.setConfig(newConfig);
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
                for (auto& [index, mdnAndSel] : m_data) {
                    mdnAndSel.first.setConfig(newConfig);
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
    const Mdn2d& num = (iter->second).first;
    if (num.name() != name) {
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


std::string mdn::Project::renameMdn(int i, const std::string& newName) {
    Mdn2d* tgt = getMdn(i);
    std::string currentName = tgt->name();
    return tgt->setName(newName);
}


std::vector<std::string> mdn::Project::toc() const {
    int nElems = size();
    if (nElems != m_addressingIndexToName.size() || nElems != m_addressingNameToIndex.size()) {
        mdn::MetaDataInvalid err("Project indexing size mismatch");
        Log_ErrorQ(err.what());
        throw err;
    }
    std::vector<std::string> result(nElems);
    std::vector<int> check(nElems, 0);
    std::string fail;
    for (const auto& [index, name] : m_addressingIndexToName) {
        if (check[index]++) {
            fail = "More than Mdn assigned to tab " + std::to_string(index);
            break;
        }
        const Mdn2d* src = getMdn(index, false);
        if (!src) {
            fail = "Could not acquire Mdn for tab " + std::to_string(index);
            break;
        }
        std::string strName = src->name();
        {
            const auto iter = m_addressingIndexToName.find(index);
            if (iter == m_addressingIndexToName.cend()) {
                fail = "Could not locate " + strName + " in the 'indexToName' data";
                break;
            }
            if (iter->second != strName) {
                fail = "Mdn '" + strName + "' at tab " + std::to_string(index) + " is incorrectly "
                    + "indexed as " + iter->second;
                break;
            }
        }
        {
            const auto iter = m_addressingNameToIndex.find(strName);
            if (iter == m_addressingNameToIndex.cend()) {
                fail = "Could not locate " + strName + " in the 'indexToName' data";
                break;
            }
            if (iter->second != index) {
                fail = "Mdn '" + strName + "' at tab " + std::to_string(index) + " is incorrectly "
                    + "indexed at tab " + std::to_string(iter->second);
                break;
            }
        }
        result[index] = src->name();
    }
    if (fail.empty()) {
        for (int i: check) {
            if (!i) {
                fail = "Tab index '" + std::to_string(i) + "' does not have a name.";
                break;
            }
        }
    }
    if (!fail.empty()) {
        mdn::MetaDataInvalid err(fail);
        Log_ErrorQ(err.what());
        throw err;
    }
    return result;
}


const mdn::Mdn2d* mdn::Project::activeMdn() const {
    return m_activeMdn2d;
}
mdn::Mdn2d* mdn::Project::activeMdn() {
    return m_activeMdn2d;
}


const mdn::Selection* mdn::Project::activeSelection() const {
    return m_activeSelection;
}
mdn::Selection* mdn::Project::activeSelection() {
    return m_activeSelection;
}


void mdn::Project::setActiveMdn(int i) {
    if (!contains(i, true)) {
        return;
    }
    auto iter = m_data.find(i);
    DBAssert(iter != m_data.end(), "Mdn is not at expected index, " << i);
    std::pair<Mdn2d, Selection> elem = iter->second;
    m_activeMdn2d = &(elem.first);
    m_activeSelection = &(elem.second);

}


void mdn::Project::setActiveMdn(std::string name) {
    int i = indexOfMdn(name);
    if (i < 0) {
        Log_WarnQ("Failed to acquire index for Mdn2d '" << name << "'");
        return;
    }
    setActiveMdn(i);
}


const std::pair<mdn::Mdn2d, mdn::Selection>* mdn::Project::at(int i, bool warnOnFailure) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "at", oss.str().c_str());
        }
        return nullptr;
    }
    return &(iter->second);
}


std::pair<mdn::Mdn2d, mdn::Selection>* mdn::Project::at(int i, bool warnOnFailure) {
    auto iter = m_data.find(i);
    if (iter == m_data.end()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "at", oss.str().c_str());
        }
        return nullptr;
    }
    return &(iter->second);
}
const std::pair<mdn::Mdn2d, mdn::Selection>* mdn::Project::at(
    std::string name,
    bool warnOnFailure
) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid Mdn name (" << name << ")";
            QMessageBox::warning(m_parent, "at", oss.str().c_str());
        }
        return nullptr;
    }
    int index = iter->second;
    return &(m_data.at(index));
}
std::pair<mdn::Mdn2d, mdn::Selection>* mdn::Project::at(std::string name, bool warnOnFailure) {
    auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            oss << "Invalid Mdn name (" << name << ")";
            QMessageBox::warning(m_parent, "at", oss.str().c_str());
        }
        return nullptr;
    }
    int index = iter->second;
    return &(m_data[index]);
}


const mdn::Selection* mdn::Project::getSelection(int i, bool warnOnFailure) const {
    return &at(i, warnOnFailure)->second;
}
mdn::Selection* mdn::Project::getSelection(int i, bool warnOnFailure) {
    return &at(i, warnOnFailure)->second;
}


const mdn::Selection* mdn::Project::getSelection(std::string name, bool warnOnFailure) const {
    return &at(name, warnOnFailure)->second;
}
mdn::Selection* mdn::Project::getSelection(std::string name, bool warnOnFailure) {
    return &at(name, warnOnFailure)->second;
}


const mdn::Mdn2d* mdn::Project::getMdn(int i, bool warnOnFailure) const {
    return &at(i, warnOnFailure)->first;
}
mdn::Mdn2d* mdn::Project::getMdn(int i, bool warnOnFailure) {
    return &at(i, warnOnFailure)->first;
}


const mdn::Mdn2d* mdn::Project::getMdn(std::string name, bool warnOnFailure) const {
    return &at(name, warnOnFailure)->first;
}
mdn::Mdn2d* mdn::Project::getMdn(std::string name, bool warnOnFailure) {
    return &at(name, warnOnFailure)->first;
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
    Selection sel;
    m_data.insert({index, {mdn, sel}});
    const std::string& origName = mdn.name();
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


std::pair<int, std::string> mdn::Project::duplicateMdn(int index) {
    Mdn2d* src = getMdn(index, true);
    if (!src) {
        return std::pair<int, std::string>(-1, "");
    }
    Mdn2d dup = Mdn2d::Duplicate(*src);
    insertMdn(dup, index + 1);
    return std::pair<int, std::string>(index+1, dup.name());
}


std::pair<int, std::string> mdn::Project::duplicateMdn(const std::string& name) {
    Mdn2d* src = getMdn(name, true);
    if (!src) {
        return std::pair<int, std::string>(-1, "");
    }
    int index = indexOfMdn(name);
    Mdn2d dup = Mdn2d::Duplicate(*src);
    insertMdn(dup, index + 1);
    return std::pair<int, std::string>(index+1, dup.name());
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
    std::string name = node.mapped().first.name();
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


bool mdn::Project::deleteMdn(int index) {
    if (!contains(index, true)) {
        return false;
    }
    std::string name = nameOfMdn(index);
    m_data.erase(index);
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    shiftMdnTabsLeft(index+1);
}


bool mdn::Project::deleteMdn(const std::string& name) {
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
    const mdn::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        return;
    }
    const Mdn2d* src = sel->get();
    if (!src || !sel->rect().isValid()) {
        return;
    }
    Rect r = sel->rect();
    // r.fixOrdering();
    encodeRectToClipboard(*src, r, QStringLiteral("rect"), QString::fromStdString(src->name()));
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
    encodeRectToClipboard(*src, b, QStringLiteral("mdn"), QString::fromStdString(src->name()));
}


void mdn::Project::cutSelection() {
    copySelection();
    deleteSelection();
}


bool mdn::Project::pasteOnSelection(int index) {
    const mdn::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        return;
    }
    Mdn2d* dst = sel->get();
    if (!dst || !sel->rect().isValid()) {
        return;
    }
    Rect r = sel->rect();
    Clipboard::DecodedPaste p = Clipboard::decodeClipboard();
    if (!p.valid()) {
        return false;
    }

    const int W = p.width();
    const int H = p.height();

    const bool haveSel = (index < 0) && sel->rect().isValid();

    int ax = 0;
    int ay = 0;

    if (haveSel) {
        const Rect r = [&]() { Rect t = sel->rect(); t.fixOrdering(); return t; }();
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
        dst->setRow(Coord(ax, ay + r), p.rows[size_t(r)]);
    }

    return true;
}


void mdn::Project::deleteSelection() {
    const mdn::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        return;
    }
    Mdn2d* dst = sel->get();
    if (!dst || !sel->rect().isValid()) {
        return;
    }
    Rect r = sel->rect();
    dst->setToZero(r);
}
