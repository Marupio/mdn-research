#pragma once

#include <vector>
#include <unordered_map>

#include "MainWindow.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

class Project: public Mdn2dFramework {

    // Null construction creates these zero-valued Mdn2d named tabs, in this order
    static const std::vector<std::string> m_defaultMdn2dNames;

    // References to the constituent Mdn2d data, key is its tab position in the gui
    std::unordered_map<int, Mdn2d> m_data;

    // m_addressingNameToIndex[name] = index
    std::unordered_map<std::string, int> m_addressingNameToIndex;

    // m_addressingIndexToName[index] = name
    std::unordered_map<int, std::string> m_addressingIndexToName;


public:

    // Construct a null project
    Project();


    // Returns the framework's derived class type name as a string
    std::string className() const override {
        return "Project";
    }

    // Returns the framework's 'name', used in error messaging
    std::string name() const override {
        return "MdnProject";
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    bool mdnNameExists(const std::string& nameIn) const override {
        // default allows name collisions
        return false;
    }

    // Gives framework final say over name changes - given a desired Mdn name change from origName
    //  to newName, returns the allowed name
    std::string requestMdnNameChange(
        const std::string& origName,
        const std::string& newName
    ) override {
        if (origName == newName) {
            return newName;
        }
        if (mdnNameExists(newName)) {
            // TODO message box
            //  "Cannot rename '" << origName << "' as '" << newName << "'. Name already exists."
            return origName;
        }

        return newName;
    }



};


} // end namespace mdn