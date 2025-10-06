#pragma once

#include <vector>

#include <mdn/Rect.hpp>
#include <mdn/Mdn2d.hpp>

namespace mdn {

class Selection {

    // Parent Mdn2d
    Mdn2d& m_ref;

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

    Selection(Mdn2d& ref): m_ref(ref), m_rect(COORD_ORIGIN) {}

    // Accessors

// TODO
// TODO: Make this class thread safe

    // Return pointer to Mdn2d to be consistent with original (StandAloneSelection)
    Mdn2d* get() { return &m_ref; }
    const Mdn2d* get() const { return &m_ref; }

    // Direct access to the reference
    Mdn2d& ref() { return m_ref; }
    const Mdn2d& ref() const { return m_ref; }

    const Rect& rect() const { return m_rect; }
    void setRect(const Rect& rectIn) {
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

    void clear() {
        m_cursor0 = m_cursor1;
        m_rect.set(m_cursor0);
    }

    inline void setPageStep(int dx, int dy) {
        m_pageDx = std::max(1, dx);
        m_pageDy = std::max(1, dy);
    }


    // * Selection queries

        // True if the selection includes a valid Mdn
        bool hasMdn() const {
            return true;
        }

        // True if the selected rectangular area is valid
        bool hasRect() const {
            return m_rect.isValid();
        }

        // True if this rect has only a single coordinate
        bool isSingleCoord() const {
            return m_rect.isSingleCoord();
        }

        // True if this rect encompasses two or more coordinates
        bool isMultiCoord() const {
            return m_rect.isMultiCoord();
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

        void cursorUp(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateY(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorDn(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateY(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorLf(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateX(-1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorRt(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateX(1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorJumpUp(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
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
            Log_Debug("extend=" << extendSelection);
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
            Log_Debug("extend=" << extendSelection);
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
            Log_Debug("extend=" << extendSelection);
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
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateY(m_pageDy);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageDn(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateY(-m_pageDy);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageLf(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateX(-m_pageDx);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPageRt(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translateX(m_pageDx);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorOrigin(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1 = COORD_ORIGIN;
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextX(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translate(1, 0);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevX(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translate(-1, 0);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorNextY(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translate(0, 1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }
        void cursorPrevY(bool extendSelection) {
            Log_Debug("extend=" << extendSelection);
            m_cursor1.translate(0, -1);
            if (extendSelection) {
                m_rect.set(m_cursor0, m_cursor1, true);
            } else {
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
            }
        }

        // Iterate cursor functions:
        //  singleCoord - moves one step in the direction specified and returns false
        //  multiCoord - at each call, increments in the direction specified, with wrapping
        //      behaviour to ensure every selected cell is visited.  Returns true if more cells need
        //      to be visited.

        bool cursorIterateX() {
            Log_Debug("");
            if (isSingleCoord()) {
                m_cursor1.translate(1, 0);
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
                return false;
            } else {
                if (m_cursor1.x() == m_rect.max().x()) {
                    if (m_cursor1.y() == m_rect.max().y()) {
                        m_cursor1 = m_rect.min();
                        return false;
                    } else {
                        m_cursor1.x() = m_rect.min().x();
                        m_cursor1.translateY(1);
                        return true;
                    }
                } else {
                    m_cursor1.translateX(1);
                    return true;
                }
            }
        }
        bool cursorIterateReverseX() {
            Log_Debug("");
            if (isSingleCoord()) {
                m_cursor1.translate(-1, 0);
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
                return false;
            } else {
                if (m_cursor1.x() == m_rect.min().x()) {
                    if (m_cursor1.y() == m_rect.min().y()) {
                        m_cursor1 = m_rect.max();
                        return false;
                    } else {
                        m_cursor1.x() = m_rect.max().x();
                        m_cursor1.translateY(-1);
                        return true;
                    }
                } else {
                    m_cursor1.translateX(-1);
                    return true;
                }
            }
        }
        bool cursorIterateY() {
            Log_Debug("");
            if (isSingleCoord()) {
                m_cursor1.translate(0, 1);
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
                return false;
            } else {
                if (m_cursor1.y() == m_rect.max().y()) {
                    if (m_cursor1.x() == m_rect.min().x()) {
                        m_cursor1 = m_rect.max();
                        return false;
                    } else {
                        m_cursor1.y() = m_rect.min().y();
                        m_cursor1.translateX(-1);
                        return true;
                    }
                } else {
                    m_cursor1.translateY(1);
                    return true;
                }
            }
        }
        bool cursorIterateReverseY() {
            Log_Debug("");
            if (isSingleCoord()) {
                m_cursor1.translate(0, -1);
                m_cursor0 = m_cursor1;
                m_rect.set(m_cursor0);
                return false;
            } else {
                if (m_cursor1.y() == m_rect.min().y()) {
                    if (m_cursor1.x() == m_rect.max().x()) {
                        m_cursor1 = m_rect.min();
                        return false;
                    } else {
                        m_cursor1.y() = m_rect.max().y();
                        m_cursor1.translateX(1);
                        return true;
                    }
                } else {
                    m_cursor1.translateY(-1);
                    return true;
                }
            }
        }

};

} // end namespace mdn
