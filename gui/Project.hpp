#pragma once

#include <vector>
#include <unordered_map>
#include <utility>

// QT includes
#include <QMessageBox>

#include "Selection.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/Mdn2dFramework.hpp"

namespace mdn {
namespace gui {
class MainWindow;
}
}


namespace mdn {
namespace gui {

class Project: public Mdn2dFramework {

protected:

    // *** Protected member data

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // The parent main window app in Qt
    MainWindow* m_parent;

    // Name of this project
    std::string m_name;

    // Config for all numbers in this project
    Mdn2dConfig m_config;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, std::pair<Mdn2d, Selection>> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Current active tab data
    std::pair<Mdn2d, Selection>* m_activeEntry;


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start', ending at 'end', shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int end=-1, int shift=1);
    void shiftMdnTabsLeft(int start, int end=-1, int shift=1);
    // void shiftMdnTabsRight(int start, int shift);
    // void shiftMdnTabsLeft(int end, int shift);


public:

    // *** Constructors

    // Construct a null project' given its name and the number of empty Mdns to start with
    Project(MainWindow* parent=nullptr, std::string name="", int nStartMdn=3);


    // *** Mdn2dFramework API

        // Returns the framework's derived class type name as a string
        inline std::string className() const override {
            return "Project";
        }

        // Returns the framework's 'name', used in error messaging
        inline std::string name() const override {
            return m_name;
        }

        // Set the project name
        inline void setName(const std::string& nameIn) override {
            m_name = nameIn;
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


    // *** Interaction with MainWindow parent

        // Apply changes in selection to MainWindow
        void updateSelection() const;

        // Direct access to underlying data
        const std::unordered_map<int, std::pair<Mdn2d, Selection>>& data_data() const {
            return m_data;
        }
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

        // Setter for m_config requires resetting of the Mdn2d's
        void setConfig(Mdn2dConfig newConfig);


        // *** MDN Accessors

        // Checks if the Mdn2d exists in the main m_data array, returns:
        //  true  - number exists
        //  false - number does not exist
        //  false - addressing data bad
        // warnOnFailure, when true, issues a QMessageBox warning if the Mdn2d is missing
        bool contains(std::string name, bool warnOnFailure = false) const;
        bool contains(int i, bool warnOnFailure = false) const;

        // Return the index (tab position) for the Mdn of the given name, -1 = not found
        int indexOfMdn(std::string name) const;

        // Return the name for the Mdn at the given tab index, empty string for bad index
        std::string nameOfMdn(int i) const;

        // Change the name of the given mdn tab
        std::string renameMdn(int i, const std::string& newName);

        // Number of Mdn tabs
        inline int size() const { return m_data.size(); }

        // Return a toc of Mdn names, listed by tab index
        //  This function also checks the metadata all agree
        std::vector<std::string> toc() const;

        const std::pair<Mdn2d, Selection>* activeEntry() const;
        std::pair<Mdn2d, Selection>* activeEntry();

        const Mdn2d* activeMdn() const;
        Mdn2d* activeMdn();

        const Selection* activeSelection() const;
        Selection* activeSelection();

        // Given mdn tab is now active
        void setActiveMdn(int i);
        void setActiveMdn(std::string name);

        // Get selection associated with i'th tab
        const Selection* getSelection(int i, bool warnOnFailure=false) const;
        Selection* getSelection(int i, bool warnOnFailure=false);

        // Get an entry of contained data (Mdn2d and Selection together)
        const std::pair<Mdn2d, Selection>* at(int i, bool warnOnFailure=false) const;
        std::pair<Mdn2d, Selection>* at(int i, bool warnOnFailure=false);
        const std::pair<Mdn2d, Selection>* at(std::string name, bool warnOnFailure=false) const;
        std::pair<Mdn2d, Selection>* at(std::string name, bool warnOnFailure=false);

        // Get selection associated with given tab name
        const Selection* getSelection(std::string name, bool warnOnFailure=false) const;
        Selection* getSelection(std::string name, bool warnOnFailure=false);

        // Return pointer to the i'th Mdn tab, nullptr on failure
        //  e.g.:
        //      Mdn2d* src = getMdn(fromIndex);
        //      AssertQ(src, "Failed to acquire Mdn2d from index " << fromIndex);
        const Mdn2d* getMdn(int i, bool warnOnFailure=false) const;
        Mdn2d* getMdn(int i, bool warnOnFailure=false);

        // Return pointer to the Mdn tab with the given name, nullptr on failure
        const Mdn2d* getMdn(std::string name, bool warnOnFailure=false) const;
        Mdn2d* getMdn(std::string name, bool warnOnFailure=false);

        // Return pointer to Mdn2d at first tab, nullptr on failure
        const Mdn2d* firstMdn(bool warnOnFailure=false) const;
        Mdn2d* firstMdn(bool warnOnFailure=false);

        // Return pointer to Mdn2d at last tab, nullptr on failure
        const Mdn2d* lastMdn(bool warnOnFailure=false) const;
        Mdn2d* lastMdn(bool warnOnFailure=false);

        // Inserts a new number at the 'end', i.e. the last index
        inline void appendMdn(Mdn2d& mdn) {
            insertMdn(mdn, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void insertMdn(Mdn2d& mdn, int index);

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
            if (auto sel = activeSelection()) {
                sel->setPageStep(dxCols, dyRows);
            }
        }

        // Access current selection
        inline const Selection* selection() const {
            if (const auto sel = activeSelection()) {
                return sel;
            }
            if (size()) {
                Log_WarnQ("No tab is currently selected.");
            }
            return nullptr;
        }
        inline Selection* selection() {
            if (auto sel = activeSelection()) {
                return sel;
            }
            if (size()) {
                Log_WarnQ("No tab is currently selected.");
            }
            return nullptr;
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

    // Debug

    // Print out all tabs and status to Log_Info
    void debugShowAllTabs() const;


};

} // end namespace gui
}  // end namespace mdn
