#pragma once

// Interface class for Mdn2d number framework class, for Mdn2d to talk upwards to the framework
// within which they exist

namespace mdn {

class Mdn2dFramework {

public:

    // Returns the framework's derived class type name as a string
    virtual std::string className() const {
        return "Mdn2dFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual std::string name() const {
        return "DummyFramework";
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    virtual bool mdn2dNameExists(const std::string& nameIn) const {
        // default allows name collisions
        return false;
    }


};

static Mdn2dFramework DummyFramework;

} // end namespace mdn