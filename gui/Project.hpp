#pragma once

#include <unordered_map>
#include <utility>
#include <vector>

// QT includes
#include <QMessageBox>
#include <QObject>

#include "../library/Mdn2d.hpp"
#include "../library/Mdn2dFramework.hpp"
#include "../library/Selection.hpp"

namespace mdn {
namespace gui {
class MainWindow;
}
}


namespace mdn {
namespace gui {

class Project: public QObject, public Mdn2dFramework {
    Q_OBJECT

protected:

    // *** Protected member data

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // The parent main window app in Qt
    MainWindow* m_parent;

    // Name of this project
    std::string m_name;

    // Path to folder holding this project (only when saved)
    std::string m_path;

    // Config for all numbers in this project
    Mdn2dConfig m_config;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Current active tab data
    mutable int m_activeIndex;


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start', ending at 'end', shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int end=-1, int shift=1);
    void shiftMdnTabsLeft(int start, int end=-1, int shift=1);
    // void shiftMdnTabsRight(int start, int shift);
    // void shiftMdnTabsLeft(int end, int shift);

    // Returns true if index i is a valid entry in m_data
    bool checkIndex(int i) const;

    // Returns true if name is a valid entry in m_addressingNameToIndex, assumes current metadata
    // Consider using indexOfMdn - checkName defers to that one anyway
    bool checkName(const std::string& name) const;

public:

    // *** Constructors

    // Construct a null project' given its name and the number of empty Mdns to start with
    Project(MainWindow* parent=nullptr, std::string name="", int nStartMdn=3);

signals:
    void tabsAboutToChange();
    void tabsChanged(int currentIndex);

public:

    // *** Mdn2dFramework API

        // Returns the framework's derived class type name as a string
        inline std::string className() const override {
            return "Project";
        }

        // Returns the framework's instance 'name'
        inline std::string name() const override {
            return m_name;
        }

        // Changes the instance name
        inline void setName(const std::string& nameIn) override {
            // Do nothing
            Log_Debug("[" << nameIn << "]");
            m_name = nameIn;
        }

        // Returns the framework's instance path
        inline std::string path() const override {
            return m_path;
        }

        // Sets the framework's instance path, should be only if a path is valid
        inline void setPath(const std::string& pathIn) override {
            // Do nothing
            Log_Debug("[" << pathIn << "]");
            m_path = pathIn;
        }


        // Returns true if an Mdn2d exists with the given name, false otherwise
        inline bool mdnNameExists(const std::string& nameIn) const override {
            return m_addressingNameToIndex.find(nameIn) != m_addressingNameToIndex.cend();
        }

        // Gives framework final say over name changes - given a desired Mdn name change from
        //  origName to newName, returns the allowed name
        //  Messaging
        //      * Information: if newName already exists (i.e. failed to rename)
        //  Exceptions
        //      * InvalidArgument: if origName does not exist (i.e. no number to rename)
        // TODO: fix access
        // *** NOT FOR PUBLIC USE *** Use renameMdn instead.
        std::string requestMdnNameChange(
            const std::string& origName,
            const std::string& newName
        ) override;

        // Returns allowed name closest to suggestedName
        std::string suggestName(const std::string& likeThis) const override;


    // *** Interaction with MainWindow parent

        // Apply changes in selection to MainWindow
        void updateSelection() const;

        // Direct access to underlying data, for those who really want it
        const std::unordered_map<std::string, int>& data_addressingNameToIndex() const {
            return m_addressingNameToIndex;
        }
        const std::unordered_map<int, std::string>& data_addressingIndexToName() const {
            return m_addressingIndexToName;
        }


    // *** Project API

        // Accessors for m_config
        inline const Mdn2dConfig& config() const {
            return m_config;
        }

        // Assess the impact of changing the config to the given value
        Mdn2dConfigImpact assessConfigChange(Mdn2dConfig config) const;

        // Setter for m_config requires resetting of the Mdn2d's
        void setConfig(Mdn2dConfig config);


        // *** MDN Accessors

        inline int activeIndex() const { return m_activeIndex; }

        // Checks if the Mdn2d exists in the main m_data array, returns:
        //  true  - number exists
        //  false - number does not exist
        //  false - addressing data bad
        // warnOnFailure, when true, issues a QMessageBox warning if the Mdn2d is missing
        bool contains(std::string name) const;
        bool contains(int i) const;

        // Return the index (tab position) for the Mdn of the given name, -1 = not found
        int indexOfMdn(std::string name) const;

        // Return the name for the Mdn at the given tab index, empty string for bad index
        std::string nameOfMdn(int i) const;

        // Change the name of the given mdn tab, checks with framework first, returns actual name
        // chosen, or empty string on failure
        std::string renameMdn(int i, const std::string& newName);
        std::string renameMdn(const std::string& oldName, const std::string& newName);

        // Number of Mdn tabs
        inline int size() const { return m_data.size(); }

        // Return a toc of Mdn names, listed by tab index
        //  This function also checks the metadata all agree
        std::vector<std::string> toc() const;

        const Mdn2d* activeMdn() const;
        Mdn2d* activeMdn();

        const Selection* activeSelection() const;
        Selection* activeSelection();

        // When no mdns exist or none are selected
        void setNoActiveMdn();

        // Given mdn tab is now active
        void setActiveMdn(int i);
        void setActiveMdn(std::string name);

        // Get selection associated with i'th tab
        const Selection* getSelection(int i) const;
        Selection* getSelection(int i);

        // Get selection associated with given tab name
        const Selection* getSelection(std::string name) const;
        Selection* getSelection(std::string name);

        // Return reference to the Mdn2d on the i'th Mdn tab
        const Mdn2d* getMdn(int i) const;
        Mdn2d* getMdn(int i);

        // Return reference to Mdn2d tab with the given name, nullptr on failure
        const Mdn2d* getMdn(std::string name) const;
        Mdn2d* getMdn(std::string name);

        // Return reference to Mdn2d at first tab, nullptr on failure
        const Mdn2d* firstMdn() const;
        Mdn2d* firstMdn();

        // Return reference to Mdn2d at last tab, nullptr on failure
        const Mdn2d* lastMdn() const;
        Mdn2d* lastMdn();

        // Inserts a new number at the 'end', i.e. the last index
        inline void appendMdn(Mdn2d&& mdn) {
            Log_Debug2_H(mdn.name());
            insertMdn(std::move(mdn), -1);
            Log_Debug2_T("");
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void insertMdn(Mdn2d&& mdn, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new mdn.
        //  An empty string return indicates the operation failed
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns empty string, takes no other action
        std::pair<int, std::string> duplicateMdn(int index);
        std::pair<int, std::string> duplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        //  This is not the same as a tab moving - this is swapping the contents of tabs
        bool moveMdn(int fromIndex, int toIndex);
        bool moveMdn(const std::string& name, int toIndex);

        // Tabs have swapped position, meaning the widgets with their data pointers have moved.  No
        //  need to move data around, just need to change indexing
        void swapIndexing(int fromIndex, int toIndex);


        // deleteMdn the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool deleteMdn(int index);
        bool deleteMdn(const std::string& name);

        int addFromClipboard(
            const mdn::Mdn2d& mdnCopy,
            const Selection& selCopy,
            const std::string& nameHint
        );

    // Selection actions

        // Set page up/down and page left/right distances (in digits)
        inline void setPageStep(int dxCols, int dyRows) {
            Log_Debug3_H(
                "dxCols=" << dxCols << ",dyRows=" << dyRows << ",activeIndex=" << m_activeIndex
            );
            Selection* sel = activeSelection();
            if (!sel) {
                Log_Debug3_T("No valid selection, cannot complete task");
                return;
            }
            sel->setPageStep(dxCols, dyRows);
            Log_Debug3_T("");
        }

        // Set new selection
        // inline void setSelection(Selection s) {
        //     m_activeSelection = std::move(s);
        //     // TODO
        //     // emit signal, update UI
        // }

        // Copy whole MDN from a tab context (scope:"mdn").
        void copyMdn(int index) const;

        // Copy current grid selection (scope:"rect"). Respects bottom-left inclusive rect.
        void copySelection() const;

        // Perform a 'cut' operation on the selection - a combination of Copy and Delete
        void cutSelection();

        // Perform a paste operation, using clipboard data and the target:, where the target depends
        //  on the supplied index:
        //      if index < 0 (or missing), use m_selection for target
        //      if index >= 0, target is Mdn tab at given index
        //  Always overwrite destination, rect is anchored to the bottom-left (xmin, ymin())
        //
        //  Source scope (data on the clipboard)
        //  A) "mdn"  - defines an entire Mdn for pasting
        //              User right-clicked the Mdn tab and chose 'Copy'
        //              User chose Copy Mdn from menu
        //  B) "rect" - defines a specific area on a specific Mdn
        //              User highlighted a rectangular area and pressed Ctrl+C, Ctrl+Insert
        //              User right-clicked highlighted rectangle and selected Copy
        //              User chose Copy Selection from menu
        //
        //  Destination scope (data currently selected, m_selection)
        //  1. selection.hasMdnOnly    - target is the entire Mdn, (index >= 0)
        //              User right-clicked Mdn tab and chose 'Paste'
        //              User chose Paste Onto Mdn from menu
        //  -- selection.hasRectOnly   - invalid - need a Mdn for actual operation
        //  2. selection.hasMdnAndRect - target is the specific area on the selected Mdn
        //              User highlighted a rectangular area and pressed Ctrl+V, Shift+Insert
        //              User right-clicked highlighted rectangle and selected Paste
        //              User chose Paste Onto Selection from menu
        //
        //  A-1 - Mdn  -> Mdn       - replace entire target Mdn with source Mdn
        //  A-2 - Mdn  -> Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
        //  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
        //  B-2 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check
        //      required: if target is 1x1, paste okay, use that as bottom left anchor, otherwise
        //      the size must match exactly
        //
        // Practically speaking:
        //  A-1 - Mdn -> Mdn
        bool pasteOnSelection(int index=-1);

        // Perform 'delete' operation on the selection, does not delete the Selection object, rather
        //  deletes the data enclosed by the selection.
        void deleteSelection();

    // Binary save / load

        // Binary persistence
        bool saveToFile(const std::string& path) const;
        static std::unique_ptr<Project> loadFromFile(MainWindow* parent, const std::string& path);

        // Lower-level (stream) variants, handy for unit tests
        void saveBinary(std::ostream& out) const;
        static std::unique_ptr<Project> loadBinary(MainWindow* parent, std::istream& in);


    // ~~~ Debug

        // Validate internals - all member data is indexed correctly, throws if not
        void validateInternals(bool logResult) const;

        // Print out all tabs and status to Log_Info
        void debugShowAllTabs(std::ostream& os) const;
};

} // end namespace gui
}  // end namespace mdn
