#pragma once

#include "GlobalConfig.hpp"
#include "Logger.hpp"

// Interface class for Mdn2d number framework class, for Mdn2d to talk upwards to the framework
// within which they exist

namespace mdn {

class MDN_API Mdn2dFramework {

public:

    // Returns the framework's derived class type name as a string
    virtual std::string className() const {
        return "Mdn2dFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual std::string name() const {
        return "DummyFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual void setName(const std::string& nameIn) {
        // Do nothing
        Log_Debug("[" << nameIn << "]");
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    virtual bool mdnNameExists(const std::string& nameIn) const {
        // default allows name collisions
        Log_Debug("[" << nameIn << "]");
        return false;
    }

    // Gives framework final say over name changes - given a desired Mdn name change from origName
    //  to newName, returns the allowed name
    virtual std::string requestMdnNameChange(
        const std::string& origName,
        const std::string& newName
    ) {
        // Default always allows the name change
        Log_Debug("o:[" << origName << "],n:[" << newName << "]");
        return newName;
    }

    // Returns allowed name closest to suggestedName
    virtual std::string suggestName(
        const std::string& suggestedName
    ) const {
        // Default always likes the suggestedName
        Log_Debug("[" << suggestedName << "]");
        return suggestedName;
    }

};

static Mdn2dFramework DummyFramework;

} // end namespace mdn