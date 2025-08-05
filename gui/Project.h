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

    // Null construction creates these zero-valued Mdn2d named tabs, in this order
    static const std::vector<std::string> m_defaultMdn2dNames;

    // Appended to project name 'untitled' when no name provided
    static int m_untitledNumber;

    // Name of this project
    std::string m_name;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;


public:

    // Construct a null project
    Project(std::string name="");


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

        // Inserts a new number at the 'end', i.e. the last index
        void AppendMdn(Mdn2d& number) {
            InsertMdn(number, -1);
        }

        // Insert a new number at the given index, index == -1 means 'at the end'
        //  Messaging
        //      * Warning: if index is too big
        //          Recover: places number at the end
        //      * Warning: if number's name conflicts
        //          Recover: rename the new number, by convention
        void InsertMdn(Mdn2d& number, int index);

        // Duplicate the Mdn2d at the given index or given name, returns the name of the new number.
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


    // Full Mdn-based selections

        void CopyMdnSelection();

};


} // end namespace mdn