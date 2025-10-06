#include "Project.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QStringList>

#include <mdn/Logger.hpp>
#include <mdn/MdnException.hpp>
#include <mdn/Rect.hpp>
#include <mdn/SignConvention.hpp>
#include <mdn/Tools.hpp>

#include "Clipboard.hpp"
#include "GuiTools.hpp"
#include "MainWindow.hpp"
#include "MdnQtInterface.hpp"

#ifdef MDN_DEBUG
    #define CHECK_INDEX(i, retval) if (!checkIndex(i)) { return retval; }
    #define CHECK_NAME(name, retval) if (indexOfMdn(name) < 0) { return retval; }
#else
    #define CHECK_INDEX(i, retval) do {} while (false);
    #define CHECK_NAME(name, retval) do {} while (false);
#endif

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


bool mdn::gui::Project::checkIndex(int i) const {
    Log_Debug4_H("i=" << i);
    if (m_data.empty()) {
        Log_Debug3("Empty data");
        Log_Debug4_T("");
        return false;
    }
    int maxIndex = m_data.size() - 1;
    if (i < 0 || i > maxIndex) {
        Log_Debug3("Index out-of-range: " << i << " (0 .. " << maxIndex << ")");
        Log_Debug4_T("");
        return false;
    }
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        InvalidArgument err("Internal Error: Index is in range but not available");
        Log_ErrorQ(err.what());
        throw err;
    }
    Log_Debug4_T("index okay")
    return true;
}


bool mdn::gui::Project::checkName(const std::string& name) const {
    Log_Debug4_H("name=" << name);
    return indexOfMdn(name) >= 0;
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
        "Creating a new Project " << (parent ? "(with parent)" : "(no parent)")
            << " '" << name << "' with " << nStartMdn << " starting tabs"
    );
    if (nStartMdn < 1) {
        setNoActiveMdn();
    } else {
        for (int i = 0; i < nStartMdn; ++i) {
            Log_Debug3("Project constructor, Mdn index " << i);
            std::string nextName = Project::suggestName("Mdn0");
            Log_Debug3("Got name=[" << nextName << "], constructor dispatch");
            Mdn2d newMdn = Mdn2d::NewInstance(m_config, nextName);
            Log_Debug2("Creating Mdn {'" << nextName << "', " << i << "}");
            appendMdn(std::move(newMdn));
        }
        setActiveMdn(0);
    }
    Log_Debug_T("");
}


mdn::gui::Project::Project(MainWindow* parent, Mdn2dConfig& cfg, int nStartMdn):
    QObject(parent),
    m_parent(parent),
    m_name(cfg.parentName()),
    m_path(cfg.parentPath()),
    m_config(cfg)
{
    if (m_name.empty()) {
        m_name = "untitled-" + std::to_string(m_untitledNumber++);
    }
    Log_Debug_H(
        "Creating a new Project " << (parent ? "(with parent)" : "(no parent)")
            << " '" << m_name << "' with " << nStartMdn << " starting tabs"
    );
    m_config.setParent(*this);
    if (nStartMdn < 1) {
        setNoActiveMdn();
    } else {
        for (int i = 0; i < nStartMdn; ++i) {
            Log_Debug3("Project constructor, Mdn index " << i);
            std::string nextName = Project::suggestName("Mdn0");
            Log_Debug3("Got name=[" << nextName << "], constructor dispatch");
            Mdn2d newMdn = Mdn2d::NewInstance(m_config, nextName);
            Log_Debug2("Creating Mdn {'" << nextName << "', " << i << "}");
            appendMdn(std::move(newMdn));
        }
        setActiveMdn(0);
    }
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


std::string mdn::gui::Project::suggestName(const std::string& likeThis) const {
    Log_Debug_H(likeThis);
    std::string working = likeThis.empty() ? "Mdn0" : likeThis;
    if (!contains(working)) {
        Log_Debug_T("Returning " << working);
        return working;
    }
    std::pair<std::string, int> prefixValue(Tools::strInt(working));
    std::string prefix = prefixValue.first;
    int val = prefixValue.second + 1;
    std::string suggestion;
    do {
        suggestion = prefix + std::to_string(val++);
    } while (contains(suggestion));
    Log_Debug_T("Returning " << suggestion);
    return suggestion;
}


void mdn::gui::Project::updateSelection() const {
    if (!m_parent) {
        return;
    }
    AssertQ(false, "This function is not implemented");
    // Not yet implemented:
    // m_parent->updateSelection(m_activeMdn2d, m_activeSelection);
}


mdn::Mdn2dConfigImpact mdn::gui::Project::assessConfigChange(Mdn2dConfig config) const {
    // Assume config is the same across all Mdn2d's
    Log_Debug3_H("config=" << config);
    const Mdn2d* first = firstMdn();
    if (!first) {
        Log_Debug3_T("No Mdns available, returning Unknown");
        return Mdn2dConfigImpact::Unknown;
    }
    Mdn2dConfigImpact impact = first->assessConfigChange(config);
    Log_Debug3_T("Impact = " << Mdn2dConfigImpactToName(impact));
    return impact;
}


void mdn::gui::Project::setConfig(Mdn2dConfig config, bool ignoreSignConventionChanges) {
    Log_Debug2_H("config=" << config);
    // Ensure parent is set correctly
    config.setParent(*this);
    m_name = config.parentName();
    m_path = config.parentPath();
    if (m_data.empty()) {
        m_config = config;
        Log_Debug("Changed config to " << config);
        Log_Debug2_T("No impact");
        return;
    }

    // For now, assume config is the same across all Mdn2d's
    Mdn2d* first = firstMdn();
    if (!first) {
        Log_Debug2_T("No Mdns available, setting global config to " << config);
        m_config = config;
        return;
    }
    Mdn2dConfigImpact impact = first->assessConfigChange(config);
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
            for (auto& [index, tgt] : m_data) {
                tgt.setConfig(config);
            }
            m_config = config;
            Log_Debug("Changed config to " << config);
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
                for (auto& [index, tgt] : m_data) {
                    tgt.setConfig(config);
                }
                m_config = config;
                Log_Debug(
                    "Changing config to: " << config << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            } else {
                // User cancelled
                Log_Debug(
                    "User cancelled changing config to: " << config << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            }
        }
        case Mdn2dConfigImpact::PossiblePolymorphism: {
            if (ignoreSignConventionChanges) {
                for (auto& [index, tgt] : m_data) {
                    tgt.setConfig(config);
                }
                m_config = config;
                Log_Debug("Ignoring changed signConvention - Changed config to " << config);
                Log_Debug2_T("");
                return;
            }
            // else fallthrough
        }
        case Mdn2dConfigImpact::PossibleDigitLoss:
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
                for (auto& [index, tgt] : m_data) {
                    tgt.setConfig(config);
                }
                m_config = config;
                Log_Debug(
                    "Changing config to: " << config << ", impact: "
                        << Mdn2dConfigImpactToName(impact)
                );
                Log_Debug2_T("");
                return;
            } else {
                // User cancelled
                Log_Debug(
                    "User cancelled changing config to: " << config << ", impact: "
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


bool mdn::gui::Project::contains(std::string name) const {
    Log_Debug3_H("name='" << name << "'");
    if (m_data.empty()) {
        Log_Debug2("Data empty, does not contain '" << name << "'");
        Log_Debug3_T("");
        return false;
    }
    int index = indexOfMdn(name);
    if (index < 0) {
        Log_Debug2("Project does not contain '" << name << "'");
        Log_Debug3_T("");
        return false;
    }
    // #ifdef MDN_DEBUG
    //     const auto iter = m_data.find(index);
    //     if (iter == m_data.cend()) {
    //         Log_Debug2("Project does not contain '" << name << "', but it is in addressing.");
    //         Log_Debug3_T("");
    //         return false;
    //     }
    // #endif
    Log_Debug2("Project contains '" << name << "'");
    Log_Debug3_T("");
    return true;
}


bool mdn::gui::Project::contains(int i) const {
    Log_Debug3("i=" << i);
    return checkIndex(i);
}


int mdn::gui::Project::indexOfMdn(std::string name) const {
    Log_Debug4_H("name=" << name);
    if (m_data.empty()) {
        Log_Debug3("Empty data");
        Log_Debug4_T("");
        return -1;
    }
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        Log_Debug3("Name [" << name << "] not present in the index");
        Log_Debug4_T("");
        return -1;
    }
    Log_Debug3("found [" << name << "] at " << iter->second);
    Log_Debug4_T("");
    return iter->second;
}


std::string mdn::gui::Project::nameOfMdn(int i) const {
    Log_Debug4_H("i=" << i);
    const auto iter = m_addressingIndexToName.find(i);
    if (iter == m_addressingIndexToName.cend()) {
        Log_Debug2("Name of " << i << " (does not exist) returning empty string");
        Log_Debug4_T("");
        return "";
    }
    Log_Debug2("Name of " << i << " = '" << iter->second << "'");
    Log_Debug4_T("");
    return iter->second;
}


std::string mdn::gui::Project::renameMdn(int i, const std::string& newName) {
    Log_Debug3_H("i=" << i << ", newName=" << newName);
    Mdn2d* tgt = getMdn(i);
    if (!tgt) {
        Log_Debug3_T("Could not locate mdn at index " << i << ", failed");
        return "";
    }
    std::string currentName = tgt->name();
    std::string actualName = tgt->setName(newName);
    Log_Debug2(
        "renaming tab {'" << currentName << "', " << i << "} to "
            << "{'" << actualName << "', " << i << "} (wanted '" << newName << "')"
    );
    m_addressingNameToIndex.erase(currentName);
    m_addressingNameToIndex[actualName] = i;
    m_addressingIndexToName[i] = actualName;
    Log_Debug3_T("renaming complete, wanted: [" << newName << "], got: [" << actualName << "]");
    return actualName;
}


std::string mdn::gui::Project::renameMdn(const std::string& oldName, const std::string& newName) {
    Log_Debug3_H("from [" << oldName << "] to [" << newName << "]");
    int i = indexOfMdn(oldName);
    std::string actualName = renameMdn(i, newName);
    Log_Debug3_T("returning [" << actualName << "]");
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
        const Mdn2d* src = getMdn(index);
        if (!src) {
            fail = "Could not acquire an Mdn2d with index " + std::to_string(index);
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
        std::string strName = getMdn(index)->name();
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
    if (m_data.empty()) {
        Log_Debug2("Empty data - no Mdns, returning nullptr");
        Log_Debug3_T("");
        return nullptr;
    }
    if (m_activeIndex == -1) {
        Log_Debug2("No active Mdn, returning nullptr");
        Log_Debug3_T("");
        return nullptr;
    }
    int maxIndex = m_data.size() - 1;
    if (m_activeIndex < 0 || m_activeIndex > maxIndex) {
        std::ostringstream oss;
        oss << "Active index out-of-range: " << m_activeIndex << " (0 .. " << maxIndex << ")";
        InvalidState err(oss.str());
        Log_ErrorQ(err.what());
        throw err;
    }
    if (!contains(m_activeIndex)) {
        InvalidState err("Internal Error: Active index is not available");
        Log_ErrorQ(err.what());
        throw err;
    }
    Log_Debug3_T("");
    return &(m_data.at(m_activeIndex));
}
mdn::Mdn2d* mdn::gui::Project::activeMdn() {
    Log_Debug3_H("");
    if (m_data.empty()) {
        Log_Debug2("Empty data - no Mdns, returning nullptr");
        Log_Debug3_T("");
        return nullptr;
    }
    if (m_activeIndex == -1) {
        Log_Debug2("No active Mdn, returning nullptr");
        Log_Debug3_T("");
        return nullptr;
    }
    #ifdef MDN_DEBUG
        if (!checkIndex(m_activeIndex)) {
            int maxIndex = m_data.size() - 1;
            std::ostringstream oss;
            oss << "Active index out-of-range: " << m_activeIndex << " (0 .. " << maxIndex << ")";
            InvalidState err(oss.str());
            Log_ErrorQ(err.what());
            throw err;
        }
        if (!contains(m_activeIndex)) {
            InvalidState err("Internal Error: Active index is not available");
            Log_ErrorQ(err.what());
            throw err;
        }
    #endif
    Log_Debug3_T("");
    return &(m_data.at(m_activeIndex));
}


const mdn::Selection* mdn::gui::Project::activeSelection() const {
    Log_Debug3_H("");
    const Mdn2d* src(activeMdn());
    if (!src) {
        Log_Debug3_T("No activeMdn, therefor no active selection");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(src->selection());
}
mdn::Selection* mdn::gui::Project::activeSelection() {
    Log_Debug3_H("");
    Mdn2d* src(activeMdn());
    if (!src) {
        Log_Debug3_T("No activeMdn, therefor no active selection");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(src->selection());
}


void mdn::gui::Project::setNoActiveMdn() {
    Log_Debug4("Setting active index to -1");
    m_activeIndex = -1;
}


void mdn::gui::Project::setActiveMdn(int i) {
    Log_Debug3_H("index=" << i);
    if (!contains(i)) {
        Log_Debug2("Failed to set activeMdn to " << i << ", does not exist");
        Log_Debug3_T("");
        return;
    }
    Log_Debug4("Setting active index=" << i);
    m_activeIndex = i;
    If_Log_Showing_Debug2(
        auto iter = m_data.find(i);
        DBAssert(iter != m_data.end(), "Mdn is not at expected index, " << i);
        Log_Debug2("Set active {'" << iter->second.name() << "', " << i << "}");
    );
    Log_Debug3_T("");
}


void mdn::gui::Project::setActiveMdn(std::string name) {
    Log_Debug3_H("name='" << name << "'");
    int i = indexOfMdn(name);
    if (i < 0) {
        Log_Warn("Failed to acquire index for Mdn2d '" << name << "'");
        Log_Debug3_T("");
        return;
    }
    Log_Debug2("Setting active: {'" << name << ", " << i << "}");
    setActiveMdn(i);
    Log_Debug3_T("");
}


const mdn::Selection* mdn::gui::Project::getSelection(int i) const {
    Log_Debug3_H("" << i);
    const Mdn2d* src(getMdn(i));
    if (!src) {
        Log_Debug3_T("Index " << i << " did not give a valid Mdn2d");
        return nullptr;
    }
    Log_Debug3_T("")
    return &(src->selection());
}
mdn::Selection* mdn::gui::Project::getSelection(int i) {
    Log_Debug3_H("" << i);
    Mdn2d* src(getMdn(i));
    if (!src) {
        Log_Debug3_T("Index " << i << " did not give a valid Mdn2d");
        return nullptr;
    }
    Log_Debug3_T("")
    return &(src->selection());
}


const mdn::Selection* mdn::gui::Project::getSelection(std::string name) const {
    Log_Debug3_H(name);
    const Mdn2d* src(getMdn(name));
    if (!src) {
        Log_Debug3_T("No valid Mdn named [" << name << "]");
        return nullptr;
    }
    Log_Debug3_T("")
    return &(src->selection());
}
mdn::Selection* mdn::gui::Project::getSelection(std::string name) {
    Log_Debug3_H(name);
    Mdn2d* src(getMdn(name));
    if (!src) {
        Log_Debug3_T("No valid Mdn named [" << name << "]");
        return nullptr;
    }
    Log_Debug3_T("")
    return &(src->selection());
}


const mdn::Mdn2d* mdn::gui::Project::getMdn(int i) const {
    Log_Debug3_H("i=" << i);
    if (!checkIndex(i)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data.at(i));
}
mdn::Mdn2d* mdn::gui::Project::getMdn(int i) {
    Log_Debug3_H("i=" << i);
    if (!checkIndex(i)) {
        Log_Debug3_T("Not a valid index");
        return nullptr;
    }
    Log_Debug3_T("");
    return &(m_data[i]);
}


const mdn::Mdn2d* mdn::gui::Project::getMdn(std::string name) const {
    Log_Debug3_H(name);
    int i = indexOfMdn(name);
    if (i < 0) {
        Log_Debug3_T("Could not locate Mdn [" << name << "]");
        return nullptr;
    }
    Log_Debug3_T("returning mdn at index " << i);
    return &(m_data.at(i));
}
mdn::Mdn2d* mdn::gui::Project::getMdn(std::string name) {
    Log_Debug3_H(name);
    int i = indexOfMdn(name);
    if (i < 0) {
        Log_Debug3_T("Could not locate Mdn [" << name << "]");
        return nullptr;
    }
    Log_Debug3_T("returning mdn at index " << i);
    return &(m_data[i]);
}


const mdn::Mdn2d* mdn::gui::Project::firstMdn() const {
    Log_Debug3("");
    return getMdn(0);
}
mdn::Mdn2d* mdn::gui::Project::firstMdn() {
    Log_Debug3("");
    return getMdn(0);
}


const mdn::Mdn2d* mdn::gui::Project::lastMdn() const {
    Log_Debug3("");
    int lastI = m_data.size() - 1;
    return getMdn(lastI);
}
mdn::Mdn2d* mdn::gui::Project::lastMdn() {
    Log_Debug3("");
    int lastI = m_data.size() - 1;
    return getMdn(lastI);
}


void mdn::gui::Project::insertMdn(Mdn2d&& mdn, int index) {
    Log_Debug2_H("");
    Log_Debug("inserting tab {'" << mdn.name() << "', " << index << "}");

    Log_Debug3_H("Emitting tabsAboutToChange");
    Q_EMIT tabsAboutToChange();
    Log_Debug3_T("Done emitting tabsAboutToChange");

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
        newName = suggestCopyName(origName);
        if (warnings > 0) {
            oss << std::endl;
        }
        oss << "Mdn '" << origName << "' already exists. Renaming new Mdn to '" << newName << "'.";
        warnings += 1;
    } else {
        newName = origName;
    }
    if (!oss.str().empty()) {
        Log_Warn(oss.str());
    }

    // Shift addressing over
    if (index < maxNewIndex) {
        Log_Debug2("Shifting tabs to the right from " << index << " -->");
        shiftMdnTabsRight(index);
    }
    Log_Debug2("Inserting {'" << newName << "'," << index << "} into data");
    m_data.try_emplace(index, std::move(mdn));
    Mdn2d& newEntry = m_data[index];
    Log_Debug2("Inserting {'" << newName << "'," << index << "} into indices");
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName.insert({index, newName});
    if (contains("__mdn_Project_dummyMdn")) {
        deleteMdn("__mdn_Project_dummyMdn");
    }
    Log_Debug3_H("Emitting tabsChanged");
    Q_EMIT tabsChanged(index);
    Log_Debug3_T("Done emitting tabsChanged");

    Log_Debug2_T("");
}


bool mdn::gui::Project::importMdn(Mdn2d&& mdn, int index) {
    Mdn2dConfigImpact impact = mdn.assessConfigChange(m_config);
    if (impact == Mdn2dConfigImpact::AllDigitsCleared) {
        // Incompatible number
        return false;
    }
    std::string newName = suggestName(mdn.name());
    mdn.setName(newName);
    mdn.setConfig(m_config);
    insertMdn(std::move(mdn), index);
    return true;
}


std::pair<int, std::string> mdn::gui::Project::duplicateMdn(int index) {
    Log_Debug2_H("index=" << index);
    if (!checkIndex(index)) {
        std::pair<int, std::string> fail(-1, "");
        Log_Debug2_T("failed checkIndex, returning fail");
        return fail;
    }
    Mdn2d* src = getMdn(index);
    // Since we've done checkIndex, it's okay to assume src is valid
    DBAssertQ(src, "checkIndex said " + std::to_string(index) + " should be valid, but it is not");
    Log_Debug3("duplicating {'" << src->name() << "', " << index << "}");
    std::pair<int, std::string> result;
    Mdn2d dup = Mdn2d::Duplicate(*src);
    Log_Debug(
        "duplicating: {'" << src->name() << "', " << index << "} --> "
        "{'" << dup.name() << "', " << index+1 << "}"
    );
    insertMdn(std::move(dup), index + 1);
    Mdn2d* insertedDup = getMdn(index + 1);

    // TODO create operator= for Selections
    Selection& sel = src->selection();
    Selection& idSel = insertedDup->selection();

    idSel.setRect(sel.rect());
    idSel.setCursor0(sel.cursor0());
    idSel.setCursor1(sel.cursor1());
    result = std::pair<int, std::string>(index+1, dup.name());
    Mdn2d* check = getMdn(index + 1);
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
    std::pair<int, std::string> result = duplicateMdn(index);
    Log_Debug2_T("returning {'" << result.second << "', " << result.first << "}");
    return result;
}


bool mdn::gui::Project::moveMdn(int fromIndex, int toIndex) {
    if (fromIndex == toIndex) {
        // Nothing to do
        Log_Debug("Moving mdn from and to same index, nothing to do");
        return true;
    }
    Log_Debug2_H("from " << fromIndex << " to " << toIndex);
    if (!checkIndex(fromIndex)) {
        Log_Debug2_T("Not a valid fromIndex, returning false");
        return false;
    }

    Log_Debug3_H("Emitting tabsAboutToChange");
    Q_EMIT tabsAboutToChange();
    Log_Debug3_T("Done emitting tabsAboutToChange");

    std::string mdnName(nameOfMdn(fromIndex));
    DBAssertQ(
        !mdnName.empty(),
        "Index " + std::to_string(fromIndex) + " should be valid, but is not"
    );
    Log_Debug("Moving {'" << mdnName << "', " << fromIndex << "} to " << toIndex);
    // Extract the number and erase the addressing metadata
    Log_Debug2("Extracting {'" << mdnName << "', " << fromIndex << "}");
    auto node = m_data.extract(fromIndex);
    std::string name = node.mapped().name();
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

    Log_Debug3_H("Emitting tabsChanged");
    Q_EMIT tabsChanged(toIndex);
    Log_Debug3_T("Done emitting tabsChanged");

    Log_Debug2_T("returning true");
    return true;
}


bool mdn::gui::Project::moveMdn(const std::string& name, int toIndex) {
    Log_Debug2_H("Moving '" << name << "' to " << toIndex);
    int fromIndex = indexOfMdn(name);
    if (fromIndex < 0) {
        Log_Warn("Mdn2d with name '" << name << "' does not exist.");
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
    if (!checkIndex(index)) {
        Log_Debug2_T("Not a valid index, returning false");
        return false;
    }
    std::string name = nameOfMdn(index);
    DBAssertQ(
        !name.empty(),
        "Index " + std::to_string(index) + " should be valid, but is not"
    );
    Log_Debug("Deleting {'" << name << "', " << index << "} from data");

    Log_Debug3_H("Emitting tabsAboutToChange");
    Q_EMIT tabsAboutToChange();
    Log_Debug3_T("Done emitting tabsAboutToChange");

    m_data.erase(index);
    Log_Debug("Deleting {'" << name << "', " << index << "} from index");
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    Log_Debug2("shifting tabs from " << index << " <--");
    shiftMdnTabsLeft(index+1);

    if (m_data.empty()) {
        setNoActiveMdn();
    } else {
        int maxIndex = m_data.size()-1;
        index = std::max(index, 0);
        index = std::min(index, maxIndex);
        setActiveMdn(index);
    }

    Log_Debug3_H("Emitting tabsChanged");
    Q_EMIT tabsChanged(index);
    Log_Debug3_T("Done emitting tabsChanged");

    Log_Debug2_T("");
    return true;
}


bool mdn::gui::Project::deleteMdn(const std::string& name) {
    Log_Debug2_H("Deleting '" << name << "'");
    int index = indexOfMdn(name);
    if (index < 0) {
        Log_Warn("Mdn2d with name '" << name << "' does not exist.");
        Log_Debug2_T("Returning false");
        return false;
    }
    bool result = deleteMdn(index);
    Log_Debug2_T("returning " << result);
    return result;
}


void mdn::gui::Project::copySelection() const {
    Log_Debug2_H("");
    const Selection* sel = activeSelection();
    if (!sel || !sel->rect().isValid()) {
        Log_Debug("No valid rectangular selection, cannot copy")
        Log_Debug2_T("");
        return;
    }
    const Mdn2d& src = sel->ref();
    Rect r = sel->rect();
    Log_Debug("Copying Mdn '" << src.name() << "', " << r);
    Clipboard::encodeRectToClipboard(
        src, r, QStringLiteral("rect"), QString::fromStdString(src.name())
    );
    Log_Debug2_T("Success");
}


void mdn::gui::Project::copyMdn(int index) const {
    Log_Debug2_H("copying " << index);
    const Mdn2d* src = getMdn(index);
    if (!src) {
        Log_Debug("No valid Mdn at index " << index << ", cannot copy");
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
    Selection* sel = activeSelection();
    if (!sel) {
        Log_Debug("No valid active selection, cannot paste");
        Log_Debug2_T("Returning false");
        return false;
    }
    Mdn2d& dst = sel->ref();
    Rect selRect = sel->rect();
    Log_Debug("Destination: " << dst.name() << ", rect=" << selRect);
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
            Log_Warn(
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
            dst.clear();
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
        dst.setRow(Coord(ax, ay + rowI), p.rows[size_t(rowI)]);
    }
    Log_Debug3("emit mdnContentChanged()");
    emit mdnContentChanged();
    Log_Debug2_T("");
    return true;
}


void mdn::gui::Project::deleteSelection() {
    Log_Debug2_H("");
    Selection* sel = activeSelection();
    if (!sel) {
        Log_Debug("No valid active selection, cannot delete");
        Log_Debug2_T("");
        return;
    }
    Mdn2d& dst = sel->ref();
    Rect r = sel->rect();
    if (r.empty()) {
        deleteMdn(dst.name());
        Log_Debug2_T("Success, deleted entire Mdn");
    } else {
        CoordSet changed(dst.setToZero(r));
        If_Log_Showing_Debug3(
            std::string coordsList(mdn::Tools::setToString<Coord>(changed, ','));
            Log_Debug3_T("Zeroed coords: " << coordsList);
        );
        If_Not_Log_Showing_Debug3(
            Log_Debug2_T("Zeroed " << changed.size() << " digits");
        );
        Log_Debug3("emit mdnContentChanged()");
        emit mdnContentChanged();
    }
}


bool mdn::gui::Project::saveToFile(const std::string& path) const {
    Log_Debug2_H("path=" << path);
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        Log_Error("Failed to open for write: " << path);
        return false;
    }
    saveBinary(out);
    const bool ok = static_cast<bool>(out);
    Log_Debug2_T("");
    return ok;
}


std::unique_ptr<mdn::gui::Project> mdn::gui::Project::loadFromFile(
    MainWindow* parent, const std::string& path
) {
    Log_Debug2_H("path=" << path);
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        Log_ErrorQ("Failed to open for read: " << path);
        return nullptr;
    }
    auto proj = loadBinary(parent, in);
    Log_Debug2_T("");
    return proj;
}


void mdn::gui::Project::saveBinary(std::ostream& out) const {
    // Magic + version so we can evolve: "MDNPRJ"
    const char magic[8] = {'M','D','N','P','R','J','\0','\0'};
    out.write(magic, 8);
    const uint32_t version = 1;
    GuiTools::binaryWrite(out, version);

    // Name
    GuiTools::binaryWriteString(out, m_name);

    // Global config (store the 5 knobs your dialog manipulates)
    // base, precision, signConvention, fraxisCascadeDepth, fraxis
    GuiTools::binaryWrite(out, static_cast<int32_t>(m_config.base()));
    GuiTools::binaryWrite(out, static_cast<int32_t>(m_config.precision()));
    GuiTools::binaryWrite(out, static_cast<int32_t>(static_cast<int>(m_config.signConvention())));
    GuiTools::binaryWrite(out, static_cast<int32_t>(m_config.fraxisCascadeDepth()));
    GuiTools::binaryWrite(out, static_cast<int32_t>(static_cast<int>(m_config.fraxis())));

    // Active tab index
    GuiTools::binaryWrite(out, static_cast<int32_t>(m_activeIndex));
    Log_Debug4("Got activeIndex=" << m_activeIndex);

    // Count
    const uint32_t count = static_cast<uint32_t>(m_data.size());
    GuiTools::binaryWrite(out, count);

    // We persist tabs in ascending gui index to be stable/readable
    // We also store each tab's name explicitly from addressing maps.
    // (Project rebuilds addressing when inserting/appending.)
    std::vector<int> indices; indices.reserve(count);
    indices.reserve(count);
    for (const auto& kv : m_data) indices.push_back(kv.first);
    std::sort(indices.begin(), indices.end());

    for (int idx : indices) {
        // Tab header
        GuiTools::binaryWrite(out, static_cast<int32_t>(idx));
        auto itName = m_addressingIndexToName.find(idx);
        std::string tabName = (itName != m_addressingIndexToName.end()) ? itName->second
                                                                        : std::string{};
        GuiTools::binaryWriteString(out, tabName);

        // Payload
        const Mdn2d& num = m_data.at(idx);
        num.saveBinary(out);  // <<=== call into Mdn2d's binary saver
        if (!out) {
            Log_ErrorQ("Failed while writing Mdn2d payload for tab " << idx);
            return;
        }
    }
}


std::unique_ptr<mdn::gui::Project> mdn::gui::Project::loadBinary(
    MainWindow* parent, std::istream& in
) {
    Log_Debug3_H("");
    // Magic + version
    char magic[8] = {};
    in.read(magic, 8);
    if (std::memcmp(magic, "MDNPRJ", 6) != 0) {
        Log_ErrorQ("Bad file header; not an MDN Project file");
        Log_Debug3_T("");
        return nullptr;
    }
    uint32_t version = 0; GuiTools::binaryRead(in, version);
    if (version != 1) {
        Log_ErrorQ("Unsupported project version: " << version);
        Log_Debug3_T("")
        return nullptr;
    }

    // Name
    std::string projName = GuiTools::binaryReadString(in);

    // Config (same order as saved)
    int32_t base=10;
    int32_t precision=-1;
    int32_t sign=0;
    int32_t cascadeDepth=Mdn2dConfig::defaultFraxisCascadeDepth();
    int32_t fraxis=0;
    GuiTools::binaryRead(in, base); GuiTools::binaryRead(in, precision); GuiTools::binaryRead(in, sign); GuiTools::binaryRead(in, cascadeDepth); GuiTools::binaryRead(in, fraxis);

    // Active index
    int32_t activeIdx = 0; GuiTools::binaryRead(in, activeIdx);

    // Create an empty project (0 start tabs so we fully control content)
    Log_Debug3("Creating new project");
    std::unique_ptr<Project> proj(new Project(parent, projName, 0));
    // Build config instance and apply through the normal path (so UI/consumers stay coherent)
    {
        Mdn2dConfig cfg(
            base,
            precision,
            static_cast<mdn::SignConvention>(sign),
            cascadeDepth,
            static_cast<mdn::Fraxis>(fraxis)
        );
        proj->setConfig(cfg);
    }

    // Count
    uint32_t count = 0; GuiTools::binaryRead(in, count);

    // Read each tab, preserving indices; let each Mdn2d load itself.
    // (Matches how the constructor seeds new tabs and appendMdn wires maps.)
    for (uint32_t k = 0; k < count; ++k) {
        int32_t idx = 0; GuiTools::binaryRead(in, idx);
        std::string tabName = GuiTools::binaryReadString(in);

        Log_Debug4("Creating {'" << tabName << "'," << idx << "}");
        // Create a fresh number with project config + correct name
        Mdn2d num = Mdn2d::NewInstance(proj->m_config, tabName);

        // Load payload
        try {
            num.loadBinary(in);
        } catch (ReadError err) {
            Log_ErrorQ("Failed to read Mdn2d for tab " << idx << ": " << err.what());
            return nullptr;
        }

        // Insert at the right position (use your existing function to keep maps in sync)
        Log_Debug4("Inserting new mdn, {'" << tabName << "'," << idx << "}");
        proj->insertMdn(std::move(num), idx); // handles out-of-range
    }

    int want = activeIdx;
    if (want < 0) {
        want = 0;
    }
    if (proj->size() == 0) {
        proj->setNoActiveMdn();
    } else {
        if (want >= proj->size()) {
            want = proj->size() - 1;
        }
        proj->setActiveMdn(want);
    }
    Log_Debug3_T("");
    return proj;
}


void mdn::gui::Project::debugShowAllTabs(std::ostream& os) const {
    Log_Debug3_H("");
    Log_Debug4("Has " << m_data.size() << " entries");
    os << "-----\n";
    int count = 0;
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        Log_Debug4("count=" << count++);
        const int index = it->first;
        Log_Debug4("index=" << index);
        const Mdn2d& entry = it->second;
        const Selection& currSel = entry.selection();
        const std::string& name = entry.name();
        Log_Debug4("name=" << name);
        const int addrIndex = m_addressingNameToIndex.at(name);
        const std::string& addrName = m_addressingIndexToName.at(index);
        os << index << "\t[" << name << "]\t(" << addrIndex << ",[" << addrName << "])";
        os << "\t" << entry.config() << "\n";
    }
    Log_Debug3_T("");
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
