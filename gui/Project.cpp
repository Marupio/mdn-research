#include "Project.h"

#include "../library/MdnException.h"

int mdn::Project::m_untitledNumber = 0;

void mdn::Project::shiftMdnTabsRight(int start, int shift) {
    int lastI = m_data.size() - 1;
    if (start > lastI) {
        // Already at the end
        return;
    }
    if (shift < 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be negative."
        );
        QMessageBox::critical(m_parent, "shiftMdnTabsRight", err.what());
        throw err;
    }

    for (int tabI = lastI; tabI >= start; --tabI) {
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI + shift;
        m_addressingIndexToName.erase(tabI);

        m_addressingIndexToName.insert({newIndex, curName});
        m_addressingNameToIndex[curName] = newIndex;
    }
}


void mdn::Project::shiftMdnTabsLeft(int start, int shift) {
    int lastI = m_data.size() - 1;
    if (start > lastI) {
        // Already at the end
        return;
    }
    if (shift < 0) {
        InvalidArgument err = InvalidArgument(
            "Tab shift (" + std::to_string(shift) + ") cannot be negative."
        );
        QMessageBox::critical(m_parent, "shiftMdnTabsRight", err.what());
        throw err;
    }
    for (int tabI = shift; tabI <= lastI; ++tabI) {
        std::string curName = m_addressingIndexToName[tabI];
        int newIndex = tabI - shift;
        m_addressingIndexToName.erase(tabI);

        m_addressingIndexToName.insert({newIndex, curName});
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
            throw error;
        }
    }


    Mdn2dConfigImpact assessConfigChange(const Mdn2dConfig& newConfig);


    // Maybe I need to be a little more surgical here
    // TODO

    if (m_config.base() != newConfig.base()) {
        // Warn user that this action will clear all numbers, get yes/no
        // no --> return
        // Clear all numbers
        // ignore all other aspects of config, set to new config
    }
}


const mdn::Mdn2d* mdn::Project::GetMdn(int i) const {
    auto iter = m_data.find(i);
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
        shiftMdnTabsRight(index, 1);
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

}


bool mdn::Project::MoveMdn(int fromIndex, int toIndex) {

}


bool mdn::Project::MoveMdn(const std::string& name, int toIndex) {

}


bool mdn::Project::Erase(int index) {

}


bool mdn::Project::Erase(const std::string& name) {

}


void mdn::Project::CopySelection() const {

}


void mdn::Project::CutSelection() {

}


void mdn::Project::PasteOnSelection() {

}


void mdn::Project::DeleteSelection() {

}


