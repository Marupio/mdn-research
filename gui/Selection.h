#pragma once

#include <vector>

#include "../library/Rect.h"
#include "../library/Mdn2d.h"
#include "../library/MdnObserver.h"

namespace mdn {

class Selection : public MdnObserver {

public:

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect rect;

    bool isEmpty() const {
        return get() == nullptr || !rect.isValid();
    }

    // Attach to an MDN
    void attach(Mdn2d* m) {
        if (m_ref == m) return;
        if (m_ref) m_ref->unregisterObserver(this);
        m_ref = m;
        if (m_ref) m_ref->registerObserver(this);
    }

    // Detatch from an MDN
    void detach() {
        if (m_ref) m_ref->unregisterObserver(this);
        m_ref = nullptr;
        rect.clear();
    }

    // Destructor
    ~Selection() { if (m_ref) m_ref->unregisterObserver(this); }


    // *** MdnObserver interface

    // The observed object is being destroyed
    void farewell() override {
        MdnObserver::farewell();
        rect.clear();
    }

    void reallocating(Mdn2d* newRef) override {
        MdnObserver::reallocating(newRef);
    }

};

} // end namespace mdn
