#pragma once

#include <vector>

#include "../library/Rect.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/MdnObserver.hpp"

namespace mdn {

class Selection : public MdnObserver {

public:

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect rect;


    // * Selection queries

        // True if the selection includes a valid Mdn
        bool hasMdn() const {
            return get() != nullptr;
        }

        // True if the selected rectangular area is valid
        bool hasRect() const {
            return rect.isValid();
        }

        // True if an Mdn is selected, but there's no rectangular area
        bool hasMdnOnly() const {
            return hasMdn() && !hasRect();
        }

        // True if a rectangular area is selected, but not on a valid Mdn
        bool hasRectOnly() const {
            return !hasMdn() && hasRect();
        }

        // True if selection contains a valid rect and a valid Mdn
        bool hasMdnAndRect() const {
            return hasRect() && hasMdn();
        }

        // True if no Mdn is selected, and no rectangular area is valid
        bool isEmpty() const {
            return !hasMdn() && !hasRect();
        }


    // * Selection operations

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
