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

class MDN_API Project: public Mdn2dFramework {

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


    // *** Protected member functions

    // Shift Mdn tabs, starting at 'start' shifting a distance of 'shift' tabs
    //  Exceptions
    //      * InvalidArgument - shift cannot be negative
    void shiftMdnTabsRight(int start, int shift);
    void shiftMdnTabsLeft(int end, int shift);


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

        // Return pointer to the i'th Mdn tab, nullptr on failure
        const Mdn2d* GetMdn(int i) const;
        Mdn2d* GetMdn(int i);

        // Return pointer to Mdn2d at first tab, nullptr on failure
        const Mdn2d* FirstMdn() const;
        Mdn2d* FirstMdn();

        // Return pointer to Mdn2d at last tab, nullptr on failure
        const Mdn2d* LastMdn() const;
        Mdn2d* LastMdn();

        // Inserts a new number at the 'end', i.e. the last index
        void AppendMdn(Mdn2d& mdn) {
            InsertMdn(mdn, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new mdn, by convention
        void InsertMdn(Mdn2d& mdn, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new mdn.
        //  An empty string return indicates the operation failed
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns empty string, takes no other action
        std::string DuplicateMdn(int index);
        std::string DuplicateMdn(const std::string& name);

        // Move Mdn2d at 'fromIndex' or given name to  'toIndex'; if toIndex == -1, moves it to the
        //  end.  Returns true or false based on success of operation.
        //  Messaging
        //      * Warning: if fromIndex out of range or name does not exist
        //          Recover: returns false
        //      * Warning: if toIndex is out of range
        //          Recover: places number at the end
        bool MoveMdn(int fromIndex, int toIndex);
        bool MoveMdn(const std::string& name, int toIndex);

        // Erase the Mdn2d at the given index or given name, shifting all higher entries one lower
        //  Messaging
        //      * Warning: if index out of range or name does not exist
        //          Recover: returns false
        bool Erase(int index);
        bool Erase(const std::string& name);


    // Selection actions

        // Perform a 'copy' operation on the selection
        void CopySelection() const;

        // Perform a 'cut' operation on the selection - a combination of Copy and Delete
        void CutSelection();

        // Selection acts as anchor to paste operation
        void PasteOnSelection();

        // Perform 'delete' operation on the selection
        void DeleteSelection();

};


} // end namespace mdn