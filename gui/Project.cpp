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
#include "../library/Tools.hpp"
#include "Clipboard.hpp"
#include "MainWindow.hpp"
#include "MdnQtInterface.hpp"


int mdn::gui::Project::m_untitledNumber = 0;

void mdn::gui::Project::shiftMdnTabsRight(int start, int end, int shift) {
    Log_Debug3_H(
        "Shifting right, all tabs from " << start
            << " to " << end << " shift rightwards by " << shift
    );
    int lastI = m_data.size();
    if (end > lastI || end < 0) {
        end = lastI;
    }
    if (start > end) {
        int tmp = start;
        start = end;
        end = tmp;
    }
    Log_Debug3(
        "Sterilized inputs, shifting right, all tabs from " << start
            << " to " << end << " shift rightwards by " << shift
    );
    if (start > lastI) {
        // Already at the end
        Log_Debug3_T("No work needed");
        return;
    }
    if (shift <= 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be zero or negative."
        );
        Log_ErrorQ(err.what());
        throw err;
    }
    for (int tabI = end; tabI >= start; --tabI) {
        auto itIN = m_addressingIndexToName.find(tabI);
        if (itIN == m_addressingIndexToName.end()) {
            Log_Debug2("Could not move tab {???, " << tabI <<"}, does not exist.  Skipping.");
            continue;
        }
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI + shift;
        m_addressingIndexToName.erase(tabI);
        m_addressingIndexToName.insert({newIndex, curName});
        Log_Debug2(
            "Moving tab {'" << curName << "', " << tabI << "}, to "
                << "{'" << curName << "', " << newIndex << "}"
        );
        #ifdef MDN_DEBUG
            auto it = m_data.find(tabI);
            Assert(it != m_data.end(), "Tab " << tabI << " missing");
        #endif
        auto node = m_data.extract(tabI);
        node.key() = newIndex;
        m_data.insert(std::move(node));
        m_addressingNameToIndex[curName] = newIndex;
    }
    Log_Debug3_T("");
}


void mdn::gui::Project::shiftMdnTabsLeft(int start, int end, int shift) {
    Log_Debug3_H(
        "Shifting left, all tabs from " << start
            << " to " << end << " shift leftwards by " << shift
    );
    int lastI = m_data.size();
    if (end > lastI || end < 0) {
        end = lastI;
    }
    if (start > end) {
        int tmp = start;
        start = end;
        end = tmp;
    }
    Log_Debug3(
        "Sterilised inputs, shifting left, all tabs from " << start
            << " to " << end << " shift leftwards by " << shift
    );
    if (start > lastI) {
        // Already at the end
        Log_Debug3_T("No work needed");
        return;
    }
    if (shift <= 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be zero or negative."
        );
        Log_ErrorQ(err.what());
        throw err;
    }
    for (int tabI = start; tabI <= end; ++tabI) {
        auto itIN = m_addressingIndexToName.find(tabI);
        if (itIN == m_addressingIndexToName.end()) {
            Log_Debug2("Could not move tab {???, " << tabI <<"}, does not exist.  Skipping.");
            continue;
        }
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI - shift;
        m_addressingIndexToName.erase(tabI);
        m_addressingIndexToName.insert({newIndex, curName});
        Log_Debug2(
            "Moving tab {'" << curName << "', " << tabI << "}, to "
                << "{'" << curName << "', " << newIndex << "}"
        );

        auto node = m_data.extract(tabI);
        node.key() = newIndex;
        m_data.insert(std::move(node));

        m_addressingNameToIndex[curName] = newIndex;
    }
    Log_Debug3_T("");
}


mdn::gui::Project::Project(MainWindow* parent, std::string name, int nStartMdn):
    QObject(parent),
    m_parent(parent),
    m_name(name)
{
    if (m_name.empty()) {
        m_name = "untitled-" + std::to_string(m_untitledNumber++);
    }
    Log_Debug_H(
        "Creating a new Project" << (parent ? "(with parent)" : "(no parent)")
            << " '" << name << "' with " << nStartMdn << " starting tabs"
    );
    if (nStartMdn == 0) {
        nStartMdn += 1;
    }
    for (int i = 0; i < nStartMdn; ++i) {
        std::string nextName = Mdn2d::static_generateNextName();
        Mdn2d newMdn = Mdn2d::NewInstance(m_config, nextName);
        Log_Debug2("Creating Mdn {'" << nextName << "', " << i << "}");
        appendMdn(newMdn);
    }
    setActiveMdn(0);
    Log_Debug_T("");
}


std::string mdn::gui::Project::requestMdnNameChange(
    const std::string& origName,
    const std::string& newName
) {
    Log_Debug3_H("origName='" << origName << "',newName='" << newName << "'");
    if (origName == newName) {
        // Nothing to change
        Log_Debug3_T("Nothing to change, returning '" << newName << "'");
        return newName;
    }
    if (!mdnNameExists(origName)) {
        InvalidArgument err = InvalidArgument("No Mdn2d named '" + origName + "' exists.");
        Log_ErrorQ(err.what());
        throw err;
    }
    if (mdnNameExists(newName)) {
        std::ostringstream oss;
        Log_InfoQ(
            "Cannot rename '" << origName << "' as '" << newName << "'. Name already exists."
        );
        Log_Debug3_T("Cannot rename, returning '" << origName << "'");
        return origName;
    }
    int index = m_addressingNameToIndex[origName];
    m_addressingNameToIndex.erase(origName);
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName[index] = newName;
    Log_Debug4(
        "Renaming {'" << origName << "'," << index << "}, as "
            << "{'" << newName << "'," << index << "}"
    );

    // This is not needed here, caller does this
    // m_data[index].m_name = newName;
    Log_Debug3_T("Returning '" << newName << "'");
    return newName;
}


void mdn::gui::Project::updateSelection() const {
    if (!m_parent) {
        return;
    }
    AssertQ(false, "This function is not implemented");
    // Not yet implemented:
    // m_parent->updateSelection(m_activeMdn2d, m_activeSelection);
}


void mdn::gui::Project::setConfig(Mdn2dConfig newConfig) {
    Log_Debug2_H("newConfig=" << newConfig);
    if (m_data.empty()) {
        m_config = newConfig;
        Log_Debug("Changed config to " << newConfig);
        Log_Debug2_T("No impact");
        return;
    }
    // For now, assume config is the same across all Mdn2d's
    Mdn2d* first = firstMdn();
    AssertQ(first, "Failed to acquire firstMdn()");
    Mdn2dConfigImpact impact = first->assessConfigChange(newConfig);
    If_Log_Showing_Debug4(
        Log_Debug4(
            "Config change impact: " << Mdn2dConfigImpactToName(impact) << ", i.e. "
                << Mdn2dConfigImpactToDescription(impact)
        );
    );
    If_Not_Log_Showing_Debug4(
        Log_Debug3("Config change impact: " << Mdn2dConfigImpactToName(impact));
    );
    switch (impact) {
        case Mdn2dConfigImpact::NoImpact: {
            m_config = newConfig;
            Log_Debug("Changed config to " << newConfig);
            Log_Debug2_T("");
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
                Log_Debug(
                    "Changing config to: " << newConfig << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            } else {
                // User cancelled
                Log_Debug(
                    "User cancelled changing config to: " << newConfig << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            }
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
                Log_Debug(
                    "Changing config to: " << newConfig << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            } else {
                // User cancelled
                Log_Debug(
                    "User cancelled changing config to: " << newConfig << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
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
    Log_Debug2_T("Unexpected code branch");
}


bool mdn::gui::Project::contains(std::string name, bool warnOnFailure) const {
    Log_Debug3_H("name='" << name << "', warn=" << warnOnFailure);
    int index = indexOfMdn(name);
    if (index < 0) {
        if (warnOnFailure) {
            Log_WarnQ("Mdn2d with name '" << name << "' does not exist.");
        }
        Log_Debug2("Project does not contain '" << name << "'");
        Log_Debug3_T("");
        return false;
    }
    const auto iter = m_data.find(index);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            Log_WarnQ("Mdn2d with name '" << name << "' exists in addressing but not in data.");
        }
        Log_Debug2("Project does not contain '" << name << "', but it is in addressing.");
        Log_Debug3_T("");
        return false;
    }
    const Mdn2d& num = (iter->second).first;
    if (num.name() != name) {
        Log_Debug2("Project does not contain '" << name << "'");
        Log_Debug3_T("");
        return false;
    }
    Log_Debug2("Project contains '" << name << "'");
    Log_Debug3_T("");
    return true;
}


bool mdn::gui::Project::contains(int i, bool warnOnFailure) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            Log_WarnQ("Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1));
        }

        Log_Debug2("Project does not contain tab " << i);
        return false;
    }
    Log_Debug2("Project contains tab " << i);
    return true;
}


int mdn::gui::Project::indexOfMdn(std::string name) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        Log_Debug2("Project does not contain '" << name << "'");
        return -1;
    }
    Log_Debug2("Project does contain {'" << name << "', " << iter->second << "}");
    return iter->second;
}


std::string mdn::gui::Project::nameOfMdn(int i) const {
    const auto iter = m_addressingIndexToName.find(i);
    if (iter == m_addressingIndexToName.cend()) {
        Log_Debug2("Name of " << i << " (does not exist) returning empty string");
        return "";
    }
    Log_Debug2("Name of " << i << " = '" << iter->second << "'");
    return iter->second;
}


std::string mdn::gui::Project::renameMdn(int i, const std::string& newName) {
    Log_Debug3_H("i=" << i << ", newName=" << newName);
    Mdn2d* tgt = getMdn(i);
    std::string currentName = tgt->name();
    std::string actualName = tgt->setName(newName);
    Log_Debug2(
        "renaming tab {'" << currentName << "', " << i << "} to "
            << "{'" << actualName << "', " << i << "} (wanted '" << newName << "')"
    );
    m_addressingNameToIndex.erase(currentName);
    m_addressingNameToIndex[actualName] = i;
    m_addressingIndexToName[i] = actualName;
    Log_Debug3_T("");
    return actualName;
}


std::vector<std::string> mdn::gui::Project::toc() const {
#ifdef MDN_DEBUG
    Log_Debug2_H("");
    int nElems = size();
    if (nElems != m_addressingIndexToName.size() || nElems != m_addressingNameToIndex.size()) {
        mdn::MetaDataInvalid err(
            "Project indexing size mismatch, (" +
                std::to_string(nElems) + "," +
                std::to_string(m_addressingIndexToName.size()) + "," +
                std::to_string(m_addressingNameToIndex.size()) + ")"
        );
        Log_ErrorQ(err.what());
        throw err;
    }
    std::unordered_set<std::string> nameSet;
    std::vector<std::string> result(nElems);
    std::vector<int> check(nElems, 0);
    std::string fail;
    for (const auto& [index, name] : m_addressingIndexToName) {
        if (check[index]++) {
            fail = "More than one Mdn assigned to tab " + std::to_string(index);
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
        auto it = nameSet.find(strName);
        if (it != nameSet.end()) {
            fail = "Mdn '" + strName + "' at tab " + std::to_string(index) + " is already in use "
                + " by another Mdn, possibly at tab " + std::to_string(indexOfMdn(strName));
        } else {
            nameSet.insert(strName);
            Log_Debug3("Found {'" << strName << "', " << index << "}");
            result[index] = strName;
        }
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
        Log_Debug2_T("Throwing error");
        throw err;
    }
    If_Log_Showing_Debug3(
        std::string rstr(mdn::Tools::vectorToString<std::string>(result, ',', false));
        Log_Debug3_T("returning list of names: [" << rstr << "]");
    );
    If_Not_Log_Showing_Debug3(
        Log_Debug2_T("returning " << result.size() << " names");
    );
    return result;
#else
    Log_Debug2_H("");
    std::vector<std::string> result(size());
    for (const auto& [index, name] : m_addressingIndexToName) {
        std::string strName = getMdn(index, false)->name();
        result[index] = strName;
    }
    If_Log_Showing_Debug3(
        std::string rstr(mdn::Tools::vectorToString<std::string>(result, ',', false));
        Log_Debug3_T("returning list of names: [" << rstr << "]");
    );
    If_Not_Log_Showing_Debug3(
        Log_Debug2_T("returning " << result.size() << " names");
    );
    return result;
#endif
}


const mdn::Mdn2d* mdn::gui::Project::activeMdn() const {
    Log_Debug3_H("");
    if (!contains(m_activeIndex)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data.at(m_activeIndex).first);
}
mdn::Mdn2d* mdn::gui::Project::activeMdn() {
    Log_Debug3_H("");
    if (!contains(m_activeIndex)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data[m_activeIndex].first);
}


const mdn::gui::Selection* mdn::gui::Project::activeSelection() const {
    Log_Debug3_H("");
    if (!contains(m_activeIndex)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data.at(m_activeIndex).second);
}
mdn::gui::Selection* mdn::gui::Project::activeSelection() {
    Log_Debug3_H("Here, checking for " << m_activeIndex);
    Log_Info("Checking if it contains " << m_activeIndex);
    bool containsIt = contains(m_activeIndex);
    Log_Info("The repsonse was: " << containsIt);
    if (!contains(m_activeIndex)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data[m_activeIndex].second);
}


void mdn::gui::Project::setActiveMdn(int i) {
    Log_Debug3_H("index=" << i);
    if (!contains(i, true)) {
        Log_Debug2("Failed to set activeMdn to " << i << ", does not exist");
        Log_Debug3_T("");
        return;
    }
    m_activeIndex = i;
    If_Log_Showing_Debug2(
        auto iter = m_data.find(i);
        DBAssert(iter != m_data.end(), "Mdn is not at expected index, " << i);
        Log_Debug2("Set active {'" << iter->second.first.name() << "', " << i << "}");
    );
    Log_Debug3_T("");
}


void mdn::gui::Project::setActiveMdn(std::string name) {
    Log_Debug3_H("name='" << name << "'");
    int i = indexOfMdn(name);
    if (i < 0) {
        Log_WarnQ("Failed to acquire index for Mdn2d '" << name << "'");
        Log_Debug3_T("");
        return;
    }
    Log_Debug2("Setting active: {'" << name << ", " << i << "}");
    setActiveMdn(i);
    Log_Debug3_T("");
}


const std::pair<mdn::Mdn2d, mdn::gui::Selection>* mdn::gui::Project::at(
    int i,
    bool warnOnFailure
) const {
    Log_Debug4_H("i=" << i << ", warnOnFailure=" << warnOnFailure);
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnOnFailure) {
            Log_WarnQ("Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1));
        }
        Log_Debug3("tab " << i << " is (*nullptr*)");
        Log_Debug4_T("");
        return nullptr;
    }
    If_Log_Showing_Debug3(
        Log_Debug3("Returning {'" << iter->second.first.name() << "', " << i << "}");
    );
    Log_Debug4_T("");
    return &(iter->second);
}


std::pair<mdn::Mdn2d, mdn::gui::Selection>* mdn::gui::Project::at(int i, bool warnOnFailure) {
    Log_Debug4_H("i=" << i << ", warnOnFailure=" << warnOnFailure);
    auto iter = m_data.find(i);
    if (iter == m_data.end()) {
        if (warnOnFailure) {
            Log_WarnQ("Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1));
        }
        Log_Debug3("tab " << i << " is (*nullptr*)");
        Log_Debug4_T("");
        return nullptr;
    }
    If_Log_Showing_Debug3(
        Log_Debug3("Returning {'" << iter->second.first.name() << "', " << i << "}");
    );
    std::ostringstream oss;
    debugShowAllTabs(oss);
    Log_Debug4_T(oss.str());
    return &(iter->second);
}
const std::pair<mdn::Mdn2d, mdn::gui::Selection>* mdn::gui::Project::at(
    std::string name,
    bool warnOnFailure
) const {
    Log_Debug4_H("name=" << name << ", warnOnFailure=" << warnOnFailure);
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            Log_WarnQ("Invalid name (" << name << ")");
        }
        Log_Debug3("no tab '" << name << "' exists");
        Log_Debug4_T("");
        return nullptr;
    }
    int index = iter->second;
    Log_Debug3("Returning {'" << name << "', " << index << "}");
    Log_Debug4_T("");
    return &(m_data.at(index));
}
std::pair<mdn::Mdn2d, mdn::gui::Selection>* mdn::gui::Project::at(
    std::string name,
    bool warnOnFailure
) {
    Log_Debug4_H("name=" << name << ", warnOnFailure=" << warnOnFailure);
    auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        if (warnOnFailure) {
            std::ostringstream oss;
            Log_WarnQ("Invalid Mdn name (" << name << ")");
            QMessageBox::warning(m_parent, "at", oss.str().c_str());
        }
        Log_Debug3("no tab '" << name << "' exists");
        Log_Debug4_T("");
        return nullptr;
    }
    int index = iter->second;
    Log_Debug3("Returning {'" << name << "', " << index << "}");
    Log_Debug4_T("");
    return &(m_data[index]);
}


const mdn::gui::Selection* mdn::gui::Project::getSelection(int i, bool warnOnFailure) const {
    Log_Debug3("" << i << ", warnOnFailure=" << warnOnFailure);
    return &at(i, warnOnFailure)->second;
}
mdn::gui::Selection* mdn::gui::Project::getSelection(int i, bool warnOnFailure) {
    Log_Debug3("");
    return &at(i, warnOnFailure)->second;
}


const mdn::gui::Selection* mdn::gui::Project::getSelection(
    std::string name,
    bool warnOnFailure
) const {
    Log_Debug3("");
    return &at(name, warnOnFailure)->second;
}
mdn::gui::Selection* mdn::gui::Project::getSelection(std::string name, bool warnOnFailure) {
    Log_Debug3("");
    return &at(name, warnOnFailure)->second;
}


const mdn::Mdn2d* mdn::gui::Project::getMdn(int i, bool warnOnFailure) const {
    Log_Debug3("");
    return &at(i, warnOnFailure)->first;
}
mdn::Mdn2d* mdn::gui::Project::getMdn(int i, bool warnOnFailure) {
    Log_Debug3("");
    return &at(i, warnOnFailure)->first;
}


const mdn::Mdn2d* mdn::gui::Project::getMdn(std::string name, bool warnOnFailure) const {
    Log_Debug3("");
    return &at(name, warnOnFailure)->first;
}
mdn::Mdn2d* mdn::gui::Project::getMdn(std::string name, bool warnOnFailure) {
    Log_Debug3("");
    return &at(name, warnOnFailure)->first;
}


const mdn::Mdn2d* mdn::gui::Project::firstMdn(bool warnOnFailure) const {
    Log_Debug3("");
    return getMdn(0, warnOnFailure);
}
mdn::Mdn2d* mdn::gui::Project::firstMdn(bool warnOnFailure) {
    Log_Debug3("");
    return getMdn(0, warnOnFailure);
}


const mdn::Mdn2d* mdn::gui::Project::lastMdn(bool warnOnFailure) const {
    Log_Debug3("");
    int lastI = m_data.size() - 1;
    return getMdn(lastI, warnOnFailure);
}
mdn::Mdn2d* mdn::gui::Project::lastMdn(bool warnOnFailure) {
    Log_Debug3("");
    int lastI = m_data.size() - 1;
    return getMdn(lastI, warnOnFailure);
}


void mdn::gui::Project::insertMdn(Mdn2d& mdn, int index) {
    Log_Debug2_H("");
    Log_Debug("inserting tab {'" << mdn.name() << "', " << index << "}");
    Q_EMIT tabsAboutToChange();

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
    const std::string& origName = mdn.name();
    std::string newName;
    if (mdnNameExists(origName)) {
        newName = Mdn2d::static_generateCopyName(origName);
        if (warnings > 0) {
            oss << std::endl;
        }
        oss << "Mdn '" << origName << "' already exists. Renaming new Mdn to '" << newName << "'.";
        warnings += 1;
    } else {
        newName = origName;
    }
    if (!oss.str().empty()) {
        Log_WarnQ(oss.str());
    }

    // Shift addressing over
    if (index < maxNewIndex) {
        Log_Debug2("Shifting tabs to the right from " << index << " -->");
        shiftMdnTabsRight(index);
    }
    Log_Debug2("Inserting {'" << newName << "'," << index << "} into data");
    Selection sel;
    m_data.try_emplace(index, std::move(mdn), std::move(sel));
    Log_Debug2("Inserting {'" << newName << "'," << index << "} into indices");
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName.insert({index, newName});
    Q_EMIT tabsChanged(index);
    Log_Debug2_T("");
}


std::pair<int, std::string> mdn::gui::Project::duplicateMdn(int index) {
    Log_Debug2_H("index=" << index);
    Mdn2d* src = getMdn(index, true);
    if (!src) {
        Log_Debug("Cannot duplicate mdn at index " << index << ", not a valid index");
        Log_Debug2_T("");
        return std::pair<int, std::string>(-1, "");
    }
    Log_Debug("duplicating {'" << src->name() << "', " << index << "}");
    Mdn2d dup = Mdn2d::Duplicate(*src);
    Log_Debug("inserting new mdn {'" << dup.name() << "', " << index+1 << "}");
    insertMdn(dup, index + 1);
    std::pair<int, std::string> result(index+1, dup.name());
    Log_Debug2_T("Returning {'" << result.second << "', " << result.first << "}");
    return result;
}


std::pair<int, std::string> mdn::gui::Project::duplicateMdn(const std::string& name) {
    Log_Debug2_H("Duplicating '" << name << "'");
    int index = indexOfMdn(name);
    if (index < 0) {
        Log_Debug("Cannot duplicate mdn '" << name << "', not a valid name");
        Log_Debug2_T("");
        return std::pair<int, std::string>(-1, "");
    }
    Log_Debug2_T("");
    return duplicateMdn(index);
}


bool mdn::gui::Project::moveMdn(int fromIndex, int toIndex) {
    if (fromIndex == toIndex) {
        // Nothing to do
        Log_Debug("Moving mdn from and to same index, nothing to do");
        return true;
    }
    Log_Debug2_H("from " << fromIndex << " to " << toIndex);
    if (!contains(fromIndex, true)) {
        Log_Debug2_T("Not a valid fromIndex, returning false");
        return false;
    }
    Q_EMIT tabsAboutToChange();
    std::string mdnName(nameOfMdn(fromIndex));
    Log_Debug(
        "Moving {'" << mdnName << "', " << fromIndex << "} to " << toIndex
    );
    // Extract the number and erase the addressing metadata
    Log_Debug2("Extracting {'" << mdnName << "', " << fromIndex << "}");
    auto node = m_data.extract(fromIndex);
    std::string name = node.mapped().first.name();
    node.key() = toIndex;
    m_addressingIndexToName.erase(fromIndex);
    m_addressingNameToIndex.erase(name);
    if (fromIndex > toIndex) {
        // Shifting digits to the right
        Log_Debug2("shifting tabs " << (fromIndex-1) << " --> " << toIndex);
        shiftMdnTabsRight(fromIndex - 1, toIndex);
    } else {
        Log_Debug2("shifting tabs from " << (fromIndex+1) << " <-- " << toIndex);
        shiftMdnTabsLeft(fromIndex + 1, toIndex);
    }
    Log_Debug2("Inserting {'" << mdnName << "'," << toIndex << "} into data");
    m_data.insert(std::move(node));
    Log_Debug2("Inserting {'" << mdnName << "'," << toIndex << "} into indices");
    m_addressingIndexToName.insert({toIndex, name});
    m_addressingNameToIndex.insert({name, toIndex});
    Q_EMIT tabsChanged(toIndex);
    Log_Debug2_T("");
    return true;
}


bool mdn::gui::Project::moveMdn(const std::string& name, int toIndex) {
    Log_Debug2_H("Moving '" << name << "' to " << toIndex);
    int fromIndex = indexOfMdn(name);
    if (fromIndex < 0) {
        Log_WarnQ("Mdn2d with name '" << name << "' does not exist.");
        Log_Debug2_T("Returning false");
        return false;
    }
    Log_Debug(
        "Moving {'" << name << "', " << fromIndex << "} to " << toIndex
    );
    bool result = moveMdn(fromIndex, toIndex);
    Log_Debug2_T("result=" << result);
    return result;
}


bool mdn::gui::Project::deleteMdn(int index) {
    Log_Debug2_H("deleting " << index);
    if (!contains(index, true)) {
        Log_Debug2_T("Not a valid index, returning false");
        return false;
    }
    std::string name = nameOfMdn(index);
    Log_Debug("Deleting {'" << name << "', " << index << "} from data");
    Q_EMIT tabsAboutToChange();
    m_data.erase(index);
    Log_Debug("Deleting {'" << name << "', " << index << "} from index");
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    Log_Debug2("shifting tabs from " << index << " <--");
    shiftMdnTabsLeft(index+1);
    Q_EMIT tabsChanged(index);
    Log_Debug2_T("");
    return true;
}


bool mdn::gui::Project::deleteMdn(const std::string& name) {
    Log_Debug2_H("Deleting '" << name << "'");
    if (!contains(name, true)) {
        Log_Debug2_T("Not a valid name, returning false");
        return false;
    }
    int index = indexOfMdn(name);
    AssertQ(index >= 0, "Failed to find the index of contained Mdn2d '" << name << "'.");
    Log_Debug("Deleting {'" << name << "', " << index << "} from data");
    Q_EMIT tabsAboutToChange();
    m_data.erase(index);
    Log_Debug("Deleting {'" << name << "', " << index << "} from index");
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    Log_Debug2("shifting tabs from " << index << " <--");
    shiftMdnTabsLeft(index+1);
    Q_EMIT tabsChanged(index);
    Log_Debug2_T("Success, returning true");
    return true;
}


void mdn::gui::Project::copySelection() const {
    Log_Debug2_H("");
    const mdn::gui::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        Log_Debug2_T("");
        return;
    }
    const Mdn2d* src = sel->get();
    if (!src || !sel->rect().isValid()) {
        Log_Debug("Selection has no rectangular selection, cannot continue")
        Log_Debug2_T("");
        return;
    }
    Rect r = sel->rect();
    // r.fixOrdering();
    Log_Debug("Copying Mdn '" << src->name() << "', " << r);
    Clipboard::encodeRectToClipboard(
        *src, r, QStringLiteral("rect"), QString::fromStdString(src->name())
    );
    Log_Debug2_T("Success");
}


void mdn::gui::Project::copyMdn(int index) const {
    Log_Debug2_H("copying " << index);
    const Mdn2d* src = getMdn(index);
    if (src == nullptr) {
        Log_Debug("Could not copy Mdn at index " << index << ", not a valid index");
        Log_Debug2_T("");
        return;
    }
    if (!src->hasBounds()) {
        // Nothing to copy (empty MDN)
        Log_Debug("Nothing to copy, Mdn '" << src->name() << "' has no digits");
        Log_Debug2_T("");
        return;
    }

    const Rect b = src->bounds();
    Log_Debug("Copying Mdn '" << src->name() << "' with bounds " << b);
    Clipboard::encodeRectToClipboard(
        *src, b, QStringLiteral("mdn"), QString::fromStdString(src->name())
    );
    Log_Debug2_T("");
}


void mdn::gui::Project::cutSelection() {
    Log_Debug2_H("");
    Log_Debug("Cut = ->copy<- + delete");
    copySelection();
    Log_Debug("Cut = copy + ->delete<-");
    deleteSelection();
    Log_Debug2_T("");
}


bool mdn::gui::Project::pasteOnSelection(int index) {
    //  Source scope (data on the clipboard)
    //  A) "mdn"  - defines an entire Mdn for pasting
    //  B) "rect" - defines a specific area on a specific Mdn
    //
    //  Destination scope (data currently selected, m_selection)
    //  1. selection.hasMdnOnly    - target is the entire Mdn, (index >= 0)
    //  -- selection.hasRectOnly   - invalid - need a Mdn for actual operation
    //  2. selection.hasMdnAndRect - target is the specific area on the selected Mdn
    //
    //  CASES:
    //  A-1 - Mdn  -> Mdn       - replace entire target Mdn with source Mdn
    //  A-2 - Mdn  -> Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
    //  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
    //  B-2 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check
    //      required: if target is 1x1, paste okay, use that as bottom left anchor, otherwise
    //      the size must match exactly
    Log_Debug2_H("Pasting, with mdn index=" << index);
    const mdn::gui::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        Log_Debug2_T("Returning false");
        return false;
    }
    Mdn2d* dst = sel->get();
    if (!dst) { // || !sel->rect().isValid()) {
        Log_Debug2_T("No valid destination");
        return false;
    }
    Rect selRect = sel->rect();
    Clipboard::DecodedPaste p = Clipboard::decodeClipboard();
    if (!p.valid()) {
        return false;
    }

    const int W = p.width();
    const int H = p.height();

    int ax = 0;
    int ay = 0;

    // TODO - this set of nested conditionals look buggy.
    //  e.g. when clipboard data is scope type 'mdn', we are still checking for a valid rect
    if (selRect.isValid()) {
        // Destination scope 2, case = ?-2
        const int SW = selRect.width();
        const int SH = selRect.height();
        Log_Debug3("Paste Type ?-2, i.e. destination selection has a valid rect");
        if (SW == 1 && SH == 1) {
            ax = selRect.left();
            ay = selRect.bottom();
        } else if (SW == W && SH == H) {
            ax = selRect.left();
            ay = selRect.bottom();
        } else {
            // Size mismatch in grid context
            Log_WarnQ(
                "Paste data (" << SW << "x" << SH << ") incompatible with current selection ("
                    << W << "x" << H
            );
            Log_Debug2_T("Returning false");
            return false;
        }
        Log_Debug2(
            "Paste Type [?]-1, src=[???]; dst=selection(mdn), "
                    << "a(" << ax << "," << ay << ")"
        );
    } else {
        // Destination scope 1, case = ?-1
        if (p.scope == QLatin1String("rect")) {
            // Source scope B
            // ~~~ CASE B-1 ~~~
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
            Log_Debug2(
                "Paste Type B-1, src=clipboard(mdn+rect); dst=selection(mdn), "
                    << "a(" << ax << "," << ay << ")"
            );
        } else if (p.scope == QLatin1String("mdn")) {
            dst->clear();
            if (p.srcRect.isValid()) {
                ax = p.srcRect.left();
                ay = p.srcRect.bottom();
            } else {
                ax = 0;
                ay = 0;
            }
            Log_Debug2(
                "Paste Type A-1, src=clipboard(mdn); dst=selection(mdn), "
                    << "a(" << ax << "," << ay << ")"
            );
        } else {
            // Unknown scope: conservative default
            ax = 0;
            ay = 0;
        }
    }

    // Overwrite rows (zeros in payload clear cells)
    for (int rowI = 0; rowI < H; ++rowI) {
        dst->setRow(Coord(ax, ay + rowI), p.rows[size_t(rowI)]);
    }
    Log_Debug2_T("");
    return true;
}


void mdn::gui::Project::deleteSelection() {
    Log_Debug2_H("");
    const mdn::gui::Selection* sel = selection();
    if (!sel) {
        Log_WarnQ("Failed to acquire selection");
        Log_Debug2_T("No selection to delete");
        return;
    }
    Mdn2d* dst = sel->get();
    if (!dst) {
        Log_Debug2_T("No target mdn2d");
        return;
    }
    Rect r = sel->rect();
    if (r.empty()) {
        deleteMdn(dst->name());
        Log_Debug2_T("Success, deleted entire Mdn");
    } else {
        CoordSet changed(dst->setToZero(r));
        If_Log_Showing_Debug3(
            std::string coordsList(mdn::Tools::setToString<Coord>(changed, ','));
            Log_Debug3_T("Zeroed coords: " << coordsList);
        );
        If_Not_Log_Showing_Debug3(
            Log_Debug2_T("Zeroed " << changed.size() << " digits");
        );
    }
}


void mdn::gui::Project::debugShowAllTabs(std::ostream& os) const {
    os << "-----\n";
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        const int index = it->first;
        const std::pair<Mdn2d, Selection>& entry = it->second;
        const Mdn2d& currMdn = entry.first;
        const Selection& currSel = entry.second;
        const std::string& name = currMdn.name();
        const int addrIndex = m_addressingNameToIndex.at(name);
        const std::string& addrName = m_addressingIndexToName.at(index);
        os << index << "\t[" << name << "]\t(" << addrIndex << ",[" << addrName << "])\n";
    }
    os << std::endl;
}


void mdn::gui::Project::validateInternals(bool logResult) const {
    #ifdef MDN_DEBUG
        std::vector<std::string> toclist = toc();
        std::ostringstream oss;
        debugShowAllTabs(oss);
        oss << "-----\nTOC: ";
        oss << Tools::vectorToString(toclist, ",", false);
        oss << std::endl;
        if (logResult) {
            Log_Debug(oss.str());
        }
    #endif
}
