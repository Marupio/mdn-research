#pragma once

#include <regex>
#include <filesystem>

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

    // Returns the framework's instance 'name'
    virtual std::string name() const {
        Log_Warn("DummyFramework is always named 'DummyFramework'");
        return "DummyFramework";
    }

    // Changes the instance name
    virtual void setName(const std::string& nameIn) {
        // Do nothing
        Log_Warn("DummyFramework ignores name [" << nameIn << "]");
    }

    // Returns the framework's instance path
    virtual std::string path() const {
        Log_Warn("DummyFramework never has a path");
        return "";
    }

    // Sets the framework's instance path, should be only if a path is valid
    virtual void setPath(const std::string& pathIn) {
        // Do nothing
        Log_Warn("DummyFramework ignores path [" << pathIn << "]");
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    virtual bool mdnNameExists(const std::string& nameIn) const {
        // default allows name collisions
        Log_Debug("DummyFramework never reports collisions [" << nameIn << "]");
        return false;
    }

    // Gives framework final say over name changes - given a desired Mdn name change from origName
    //  to newName, returns the allowed name
    virtual std::string requestMdnNameChange(
        const std::string& origName,
        const std::string& newName
    ) {
        // Default always allows the name change
        Log_Warn(
            "DummyFramework approves changing from [" << origName << "] to [" << newName << "]"
        );
        return newName;
    }

    // Returns allowed name closest to suggestedName
    virtual std::string suggestName(
        const std::string& suggestedName
    ) const {
        // Default always likes the suggestedName
        Log_Warn("Using DummyFramework, suggesting: [" << suggestedName << "]");
        return suggestedName;
    }

    virtual std::string suggestCopyName(const std::string& suggestedName) {
        Log_Debug4_H("suggestedName=[" << suggestedName << "]");
        std::regex suffixRegex(R"(^(.*_Copy)(\d+)$)");
        std::smatch match;

        std::string candidate;
        if (std::regex_match(suggestedName, match, suffixRegex)) {
            Log_Debug4("Has suffix '_Copy#'");
            std::string base = match[1];
            int nCopy = std::stoi(match[2]) + 1;
            do {
                Log_Debug4("checking " << nCopy);
                candidate = base + std::to_string(nCopy++);
            } while (mdnNameExists(candidate));
            Log_Debug4_T("returning " << candidate);
            return candidate;
        }

        candidate = suggestedName + "_Copy0";
        Log_Debug4("No '_Copy_#' suffix, checking ..._Copy0 availability");
        while (mdnNameExists(candidate)) {
            Log_Debug4("'__Copy0' not available");
            static std::regex baseRegex(R"((.*_Copy))");
            std::smatch baseMatch;
            if (std::regex_match(candidate, baseMatch, baseRegex)) {
                int n = 1;
                do {
                    Log_Debug4("Checking " << n);
                    candidate = baseMatch[1].str() + std::to_string(n++);
                } while (mdnNameExists(candidate));
            } else {
                break;
            }
        }
        Log_Debug4_T("Returning " << candidate);
        return candidate;
    }

};

static Mdn2dFramework DummyFramework;

} // end namespace mdn