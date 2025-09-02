#pragma once

#include "Logger.hpp"

namespace mdn {

class Mdn2d;

// Hold a reference to an Mdn2d, and observe it for changes to stay up-to-date
class MdnObserver {

protected:

    // The object I am observing
    mutable Mdn2d* m_ref = nullptr;

    int m_instance = -1;


public:

    // Construct null or from reference
    MdnObserver(Mdn2d* ref=nullptr): m_ref(ref) {}

    // Return instance of this observer
    int instance() const {
        return m_instance;
    }

    // Set the instance of this observer
    void setInstance(int instance) {
        #ifdef MDN_DEBUG
            if (m_instance >= 0) {
                Log_Warn("Setting MdnObserver instance when it is already set.");
            }
        #endif
        m_instance = instance;
    }

    // Set the reference Mdn2d
    virtual void set(Mdn2d* ref) {
        m_ref = ref;
    }

    // Return the reference Mdn2d
    virtual Mdn2d* get() const {
        return m_ref;
    }

    // The observed object has been modified
    virtual void modified() const {}

    // The observed object is being reallocated to a new address
    virtual void reallocating(Mdn2d* newRef) {
        m_ref = newRef;
    }

    // The observed object is being destroyed
    virtual void farewell() {
        m_ref = nullptr;
    }

};

} // End namespace mdn
