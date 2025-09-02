#pragma once

#include <vector>

#include "../library/Rect.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/MdnObserver.hpp"

namespace mdn {
namespace gui {

class Selection : public MdnObserver {

    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    Rect m_rect;

    // Coordinate locations of the cursor:
    //  * cursor0 is where the cursor started the box selection
    //  * cursor1 is the cursor's current active position
    // These cursor positions are ALWAYS corners of rect, but not necessarily min and max
    Coord m_cursor0;
    Coord m_cursor1;

    // columns per page (>=1)
    int m_pageDx = 1;

    // rows per page (>=1)
    int m_pageDy = 1;


public:

    Selection(Mdn2d* ref=nullptr): MdnObserver(ref) {}

    // Accessors
    const Rect& rect() const { return m_rect; }
    void setRect(Rect& rectIn) {
        m_rect = rectIn;
        m_cursor0 = m_rect.min();
        m_cursor1 = m_rect.max();
    }

    // Recompute the selection rectangle from cursor0 and cursor1
    inline void syncRectToCursors() {
        // Definitely need 'true' here to fixOrdering
        m_rect.set(m_cursor0, m_cursor1, true);
    }

    const Coord& cursor0() const { return m_cursor0; }
    const Coord& cursor1() const { return m_cursor1; }

    void setCursor0(const Coord& xy) { m_cursor0 = xy; }
    void setCursor1(const Coord& xy) { m_cursor1 = xy; }

    inline void setPageStep(int dx, int dy) {
        m_pageDx = std::max(1, dx);
        m_pageDy = std::max(1, dy);
    }


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
            Log_Debug("cursorUp, extend=" << extendSelection);
            m_cursor1.translateY(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorDn(bool extendSelection) {
            Log_Debug("cursorDn, extend=" << extendSelection);
            m_cursor1.translateY(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorLf(bool extendSelection) {
            Log_Debug("cursorLf, extend=" << extendSelection);
            m_cursor1.translateX(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorRt(bool extendSelection) {
            Log_Debug("cursorRt, extend=" << extendSelection);
            m_cursor1.translateX(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpUp(bool extendSelection) {
            Log_Debug("cursorJumpUp, extend=" << extendSelection);
            const Mdn2d* dst = get();
            if (!dst) {
                Log_Warn("Failed to acquire Mdn2d");
                return;
            }
            Coord postJump = dst->jump(m_cursor1, CardinalDirection::North);
            if (postJump == m_cursor1) {
                // No actual movement
                return;
            }
            m_cursor1 = postJump;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpDn(bool extendSelection) {
            Log_Debug("cursorJumpDn, extend=" << extendSelection);
            const Mdn2d* dst = get();
            if (!dst) {
                Log_Warn("Failed to acquire Mdn2d");
                return;
            }
            Coord postJump = dst->jump(m_cursor1, CardinalDirection::South);
            if (postJump == m_cursor1) {
                // No actual movement
                return;
            }
            m_cursor1 = postJump;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpLf(bool extendSelection) {
            Log_Debug("cursorJumpLf, extend=" << extendSelection);
            const Mdn2d* dst = get();
            if (!dst) {
                Log_Warn("Failed to acquire Mdn2d");
                return;
            }
            Coord postJump = dst->jump(m_cursor1, CardinalDirection::West);
            if (postJump == m_cursor1) {
                // No actual movement
                return;
            }
            m_cursor1 = postJump;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpRt(bool extendSelection) {
            Log_Debug("cursorJumpRt, extend=" << extendSelection);
            const Mdn2d* dst = get();
            if (!dst) {
                Log_Warn("Failed to acquire Mdn2d");
                return;
            }
            Coord postJump = dst->jump(m_cursor1, CardinalDirection::East);
            if (postJump == m_cursor1) {
                // No actual movement
                return;
            }
            m_cursor1 = postJump;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageUp(bool extendSelection) {
            Log_Debug("cursorPageUp, extend=" << extendSelection);
            // Missing information - need to find out how much a page movement is
            m_cursor1.translateY(m_pageDy);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageDn(bool extendSelection) {
            Log_Debug("cursorPageDn, extend=" << extendSelection);
            // Missing information - need to find out how much a page movement is
            m_cursor1.translateY(-m_pageDy);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageLf(bool extendSelection) {
            Log_Debug("cursorPageLf, extend=" << extendSelection);
            // Missing information - need to find out how much a page movement is
            m_cursor1.translateX(-m_pageDx);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageRt(bool extendSelection) {
            Log_Debug("cursorPageRt, extend=" << extendSelection);
            // Missing information - need to find out how much a page movement is
            m_cursor1.translateX(m_pageDx);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorOrigin(bool extendSelection) {
            Log_Debug("cursorOrigin, extend=" << extendSelection);
            m_cursor1 = COORD_ORIGIN;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextX(bool extendSelection) {
            Log_Debug("cursorNextX, extend=" << extendSelection);
            m_cursor1.translate(1, 0);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevX(bool extendSelection) {
            Log_Debug("cursorPrevX, extend=" << extendSelection);
            m_cursor1.translate(-1, 0);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextY(bool extendSelection) {
            Log_Debug("cursorNextY, extend=" << extendSelection);
            m_cursor1.translate(0, 1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevY(bool extendSelection) {
            Log_Debug("cursorPrevY, extend=" << extendSelection);
            m_cursor1.translate(0, -1);
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

} // end namespace gui
} // end namespace mdn
