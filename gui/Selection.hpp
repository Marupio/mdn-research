#pragma once

#include <vector>

#include "../library/Rect.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/MdnObserver.hpp"

namespace mdn {

class Selection : public MdnObserver {

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect m_rect;

    // Coordinate locations of the cursor:
    //  * cursor0 is where the cursor started the box selection
    //  * cursor1 is the cursor's current active position
    // These cursor positions are ALWAYS corners of rect, but not necessarily min and max
    Coord m_cursor0;
    Coord m_cursor1;

public:

    // Accessors
    const Rect& rect() const { return m_rect; }
    void setRect(Rect& rectIn) {
        m_rect = rectIn;
        m_cursor0 = m_rect.min();
        m_cursor1 = m_rect.max();
    }

    const Coord& cursor0() const { return m_cursor0; }
    const Coord& cursor1() const { return m_cursor1; }


    // * Selection queries

        // True if the selection includes a valid Mdn
        bool hasMdn() const {
            return get() != nullptr;
        }

        // True if the selected rectangular area is valid
        bool hasRect() const {
            return m_rect.isValid();
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
            m_rect.clear();
        }

        void cursorUp(bool extendSelection) {
            m_cursor1.translateY(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorDn(bool extendSelection) {
            m_cursor1.translateY(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorLf(bool extendSelection) {
            m_cursor1.translateX(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorRt(bool extendSelection) {
            m_cursor1.translateX(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpUp(bool extendSelection) {
            m_cursor1.translate();
//////////////// TODOTODO ////////////////
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpDn(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpLf(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpRt(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageUp(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageDn(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageLf(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageRt(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorOrigin(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextX(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevX(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextY(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevY(bool extendSelection) {
            m_cursor1.translate();
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }


        // Destructor
        ~Selection() { if (m_ref) m_ref->unregisterObserver(this); }


    // *** MdnObserver interface

    // The observed object is being destroyed
    void farewell() override {
        MdnObserver::farewell();
        m_rect.clear();
    }

    void reallocating(Mdn2d* newRef) override {
        MdnObserver::reallocating(newRef);
    }

};

} // end namespace mdn
