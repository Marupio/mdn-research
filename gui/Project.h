#pragma once

#include <vector>
#include <unordered_map>

// QT includes
#include <QMessageBox>

#include "MainWindow.h"

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

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
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;

    // Selection, Rect bounds of digits, and list of MDNs coser;
    Selection m_selection;


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
        std::string className() const override {
            return "Project";
        }

        // Returns the framework's 'name', used in error messaging
        std::string name() const override {
            return m_name;
        }

        // Set the project name
        void setName(const std::string& nameIn) override {
            m_name = nameIn;
        }

        // Returns true if an Mdn2d exists with the given name, false otherwise
        bool mdnNameExists(const std::string& nameIn) const override {
            return m_addressingNameToIndex.find(nameIn) != m_addressingNameToIndex.cend();
        }

        // Gives framework final say over name changes - given a desired Mdn name change from
        //  origName to newName, returns the allowed name
        //  Messaging
        //      * Information: if newName already exists (i.e. failed to rename)
        //  Exceptions
        //      * InvalidArgument: if origName does not exist (i.e. no number to rename)
        std::string requestMdnNameChange(
            const std::string& origName,
            const std::string& newName
        ) override;


    // *** Project API

        // Accessors for m_config
        const Mdn2dConfig& config() const {
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
        void appendMdn(Mdn2d& mdn) {
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
        std::string duplicateMdn(int index);
        std::string duplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        bool moveMdn(int fromIndex, int toIndex);
        bool moveMdn(const std::string& name, int toIndex);

        // eraseMdn the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool eraseMdn(int index);
        bool eraseMdn(const std::string& name);


    // Selection actions

        // Access current selection
        const Selection& selection() const { return m_selection; }

        // Set new selection
        void setSelection(Selection s) {
            m_selection = std::move(s);
            // TODO
            // emit signal, update UI
        }

        // Encodes a rectangular slice from src as mdn-selection JSON + TSV fallbacks.
        static void encodeRectToClipboard(
            const Mdn2d& src, const Rect& r, const QString& scope, const QString& originMdnName
        );

        // Copy current grid selection (scope:"rect"). Respects bottom-left inclusive rect.
        void copySelection() const;

        // Copy whole MDN from a tab context (scope:"mdn").
        void copyMdn(int index) const;

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
        //  B) "rect" - defines a specific area on a specific Mdn
        //
        //  Destination scope (data currently selected, m_selection)
        //  1. selection.hasMdnOnly    - target is the entire Mdn, (index >= 0)
        //  -- selection.hasRectOnly   - invalid - need a Mdn for actual operation
        //  2. selection.hasMdnAndRect - target is the specific area on the selected Mdn
        //
        //  A-1 - Mdn ->  Mdn       - replace entire target Mdn with source Mdn
        //  A-2 - Mdn ->  Mdn+Rect  - Not valid (user error - tell user invalid data to paste here)
        //  B-1 - Rect -> Mdn       - replace same rect (absolute) on target with source
        //  B-2 - Rect -> Mdn+Rect  - replace same rect (relative) on target with source, size check
        //      required: if target is 1x1, paste okay, use that as bottom left anchor, otherwise
        //      the size must match exactly
        void pasteOnSelection(int index=-1);

        // Perform 'delete' operation on the selection
        void deleteSelection();

};


} // end namespace mdn