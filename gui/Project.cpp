#include "Project.h"

#include <QJsonObject>

#include "../library/MdnException.h"
#include "Selection.h"


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
        AppendMdn(newMdn);
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
    Mdn2d* first = FirstMdn();
    AssertQ(first, "Failed to acquire FirstMdn()");
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


bool mdn::Project::Contains(std::string name, bool warnIfMissing) const {
    int index = IndexOfMdn(name);
    if (index < 0) {
        if (warnIfMissing) {
            std::ostringstream oss;
            oss << "Mdn2d with name '" << name << "' does not exist.";
            QMessageBox::warning(m_parent, "Contains", oss.str().c_str());
            return false;
        }
        return false;
    }
    const auto iter = m_data.find(index);
    if (iter == m_data.cend()) {
        if (warnIfMissing) {
            std::ostringstream oss;
            oss << "Mdn2d with name '" << name << "' exists in addressing but not in data.";
            QMessageBox::warning(m_parent, "Contains", oss.str().c_str());
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


bool mdn::Project::Contains(int i, bool warnIfMissing) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        if (warnIfMissing) {
            std::ostringstream oss;
            oss << "Invalid index (" << i << "), expecting 0 .. " << (m_data.size() - 1);
            QMessageBox::warning(m_parent, "Contains", oss.str().c_str());
            return false;
        }

        return false;
    }
    return true;
}


int mdn::Project::IndexOfMdn(std::string name) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        return -1;
    }
    return iter->second;
}


std::string mdn::Project::NameOfMdn(int i) const {
    const auto iter = m_addressingIndexToName.find(i);
    if (iter == m_addressingIndexToName.cend()) {
        return "";
    }
    return iter->second;
}


const mdn::Mdn2d* mdn::Project::GetMdn(int i) const {
    const auto iter = m_data.find(i);
    if (iter == m_data.cend()) {
        return nullptr;
    }
    return &(iter->second);
}
mdn::Mdn2d* mdn::Project::GetMdn(int i) {
    auto iter = m_data.find(i);
    if (iter == m_data.end()) {
        return nullptr;
    }
    return &(iter->second);
}


const mdn::Mdn2d* mdn::Project::GetMdn(std::string name) const {
    const auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        return nullptr;
    }
    int index = iter->second;
    return &(m_data.at(index));
}
mdn::Mdn2d* mdn::Project::GetMdn(std::string name) {
    auto iter = m_addressingNameToIndex.find(name);
    if (iter == m_addressingNameToIndex.cend()) {
        return nullptr;
    }
    int index = iter->second;
    return &(m_data[index]);
}


const mdn::Mdn2d* mdn::Project::FirstMdn() const {
    return GetMdn(0);
}
mdn::Mdn2d* mdn::Project::FirstMdn() {
    return GetMdn(0);
}


const mdn::Mdn2d* mdn::Project::LastMdn() const {
    int lastI = m_data.size() - 1;
    return GetMdn(lastI);
}
mdn::Mdn2d* mdn::Project::LastMdn() {
    int lastI = m_data.size() - 1;
    return GetMdn(lastI);
}


void mdn::Project::InsertMdn(Mdn2d& mdn, int index) {
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
        QMessageBox::warning(m_parent, "InsertMdn", oss.str().c_str());
    }

    // Shift addressing over
    if (index < maxNewIndex) {
        shiftMdnTabsRight(index);
    }
    m_addressingNameToIndex.insert({newName, index});
    m_addressingIndexToName.insert({index, newName});
}


std::string mdn::Project::DuplicateMdn(int index) {
    Mdn2d* src = GetMdn(index);
    if (!src) {
        std::ostringstream oss;
        oss << "Invalid index (" << index << "), expecting 0 .. " << (m_data.size() - 1);
        QMessageBox::warning(m_parent, "DuplicateMdn", oss.str().c_str());
        return "";
    }
    Mdn2d dup = Mdn2d::Duplicate(*src);
    InsertMdn(dup, index + 1);
    return dup.getName();
}


std::string mdn::Project::DuplicateMdn(const std::string& name) {
    if (!Contains(name, true)) {
        return false;
    }
    Mdn2d* src = GetMdn(name);
    AssertQ(src, "Failed to retrieve Mdn '" << name << "'");
    int index = IndexOfMdn(name);
    Mdn2d dup = Mdn2d::Duplicate(*src);
    InsertMdn(dup, index + 1);
    return dup.getName();
}


bool mdn::Project::MoveMdn(int fromIndex, int toIndex) {
    if (fromIndex == toIndex) {
        // Nothing to do
        return true;
    }
    if (!Contains(fromIndex, true)) {
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


bool mdn::Project::MoveMdn(const std::string& name, int toIndex) {
    int fromIndex = IndexOfMdn(name);
    if (fromIndex < 0) {
        std::ostringstream oss;
        oss << "Mdn2d with name '" << name << "' does not exist.";
        QMessageBox::warning(m_parent, "MoveMdn", oss.str().c_str());
        return false;
    }
    return MoveMdn(fromIndex, toIndex);
}


bool mdn::Project::Erase(int index) {
    if (!Contains(index, true)) {
        return false;
    }
    std::string name = NameOfMdn(index);
    m_data.erase(index);
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    shiftMdnTabsLeft(index+1);
}


bool mdn::Project::Erase(const std::string& name) {
    if (!Contains(name, true)) {
        return false;
    }
    int index = IndexOfMdn(name);
    AssertQ(index >= 0, "Failed to find the index of contained Mdn2d '" << name << "'.");
    m_data.erase(index);
    m_addressingIndexToName.erase(index);
    m_addressingNameToIndex.erase(name);
    shiftMdnTabsLeft(index+1);
}


void mdn::Project::CopySelection() const {
    // TODO
}


void mdn::Project::CutSelection() {

}


void mdn::Project::PasteOnSelection() {

}


void mdn::Project::DeleteSelection() {

}


