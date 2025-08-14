#pragma once

#include "Coord.h"
#include "GlobalConfig.h"
#include <vector>
#include <algorithm>
#include <limits>

namespace mdn {

class MDN_API Rect {

public:

    // *** Static member functions

    // Returns an invalid rectangle, plays nice with growToInclude
    static Rect GetInvalid() {
        return Rect(
            Coord(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
            Coord(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())
        );
    }

    // Return intersection of two rects. If they don't intersect, returns Rect::GetInvalid()
    inline static Rect Intersection(const Rect& a, const Rect& b) {
        Coord minPt(
            std::max(a.min().x(), b.min().x()),
            std::max(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::min(a.max().x(), b.max().x()),
            std::min(a.max().y(), b.max().y())
        );

        Rect result(minPt, maxPt);
        return result.isValid() ? result : Rect::GetInvalid();
    }

    // Return the smallest rect that contains both input rects
    inline static Rect UnionOf(const Rect& a, const Rect& b) {
        if (!a.isValid()) return b;
        if (!b.isValid()) return a;

        Coord minPt(
            std::min(a.min().x(), b.min().x()),
            std::min(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::max(a.max().x(), b.max().x()),
            std::max(a.max().y(), b.max().y())
        );

        return Rect(minPt, maxPt);
    }

    // Check if rects overlap (intersection is non-empty)
    inline static bool Overlaps(const Rect& a, const Rect& b) {
        return Intersection(a, b).isValid();
    }

    // Check if two rects are adjacent but not overlapping
    inline static bool AreAdjacent(const Rect& a, const Rect& b) {
        if (Overlaps(a, b)) return false;

        // One of the edges must touch
        bool horizontalTouch =
            (a.right() + 1 == b.left() || b.right() + 1 == a.left()) &&
            !(a.top() < b.bottom() || b.top() < a.bottom());

        bool verticalTouch =
            (a.top() + 1 == b.bottom() || b.top() + 1 == a.bottom()) &&
            !(a.right() < b.left() || b.right() < a.left());

        return horizontalTouch || verticalTouch;
    }


    // *** Constructors

    // Construct null - retuns an invalid rectangle
    Rect() : Rect(GetInvalid()) {}

    // Construct from components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(Coord min, Coord max, bool fixOrdering = false)
        : m_min(min), m_max(max)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    // Construct from elemental components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false)
        : m_min(xmin, ymin), m_max(xmax, ymax)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }


    // *** Rule of five
    //  Compliant by default

        Rect(const Rect&) = default;
        Rect(Rect&&) = default;
        Rect& operator=(const Rect&) = default;
        Rect& operator=(Rect&&) = default;
        ~Rect() = default;


    // *** Public member functions

    // Returns true if this Rect is valid, i.e. min and max make sense
    bool isValid() const {
        return m_min.x() <= m_max.x() && m_min.y() <= m_max.y();
    }

    // Returns true if this Rect is invalid, i.e. min and max do not make sense
    bool isInvalid() const {
        return !isValid();
    }

    // Treats each component independently, ensures min and max are in the correct variable
    void fixOrdering() {
        if (m_min.x() > m_max.x()) std::swap(m_min.x(), m_max.x());
        if (m_min.y() > m_max.y()) std::swap(m_min.y(), m_max.y());
    }

    Coord min() const { return m_min; }
    Coord max() const { return m_max; }

    int left()   const { return m_min.x(); }
    int right()  const { return m_max.x(); }
    int bottom() const { return m_min.y(); }
    int top()    const { return m_max.y(); }

    int width()  const { return m_max.x() - m_min.x() + 1; }
    int height() const { return m_max.y() - m_min.y() + 1; }

    // A mathematical vector from min to max, gives (nColumns-1, nRows-1)
    Coord delta() const { return m_max - m_min; }

    // Count for number of elements along each dimension, gives (nColumns, nRows)
    Coord gridSize() const { return Coord(width(), height()); }

    // Returns the total number of integer Coordinates covered by this object, does not count non-
    //  zeros.
    int size() const {
        return isValid() ? width() * height() : 0;
    }

    bool isSingleCoord() const {
        return isValid() && m_min == m_max;
    }

    bool isMultiCoord() const {
        return isValid() && m_min != m_max;
    }

    bool contains(const Coord& c) const {
        return c.x() >= m_min.x() && c.x() <= m_max.x()
            && c.y() >= m_min.y() && c.y() <= m_max.y();
    }

    void clear() {
        *this = Rect::GetInvalid();
    }

    void set(Coord min, Coord max, bool fixOrdering = false) {
        m_min = min;
        m_max = max;
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void set(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false) {
        m_min.set(xmin, ymin);
        m_max.set(xmax, ymax);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMin(int x, int y, bool fixOrdering = false) {
        m_min.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMax(int x, int y, bool fixOrdering = false) {
        m_max.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    void translate(int dx, int dy) {
        m_min.translate(dx, dy);
        m_max.translate(dx, dy);
    }

    Rect translated(int dx, int dy) const {
        return Rect(m_min.translated(dx, dy), m_max.translated(dx, dy));
    }

    void growToInclude(const Coord& c) {
        if (!isValid()) {
            m_min = m_max = c;
            return;
        }
        if (c.x() < m_min.x()) m_min.x() = c.x();
        if (c.x() > m_max.x()) m_max.x() = c.x();
        if (c.y() < m_min.y()) m_min.y() = c.y();
        if (c.y() > m_max.y()) m_max.y() = c.y();
    }

    std::vector<Coord> toCoordVector() const {
        std::vector<Coord> coords;
        if (!isValid()) return coords;
        for (int y = bottom(); y <= top(); ++y) {
            for (int x = left(); x <= right(); ++x) {
                coords.emplace_back(x, y);
            }
        }
        return coords;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        if (!r.isValid()) {
            os << "[Empty]";
        } else {
            os << "[" << r.min() << " -> " << r.max() << "]";
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Rect& r) {
        char ch;
        is >> ch;

        if (ch != '[') {
            is.setstate(std::ios::failbit);
            return is;
        }

        // Peek ahead to see if it's "Empty]"
        std::string word;
        is >> word;

        if (word == "Empty]") {
            r = Rect::GetInvalid();
            return is;
        }

        // Back up stream to parse normally
        is.putback(word.back());
        for (int i = static_cast<int>(word.size()) - 2; i >= 0; --i)
            is.putback(word[i]);

        Coord min, max;
        is >> min;

        is >> ch; // read '-'
        if (ch != '-') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> ch; // expect '>'
        if (ch != '>') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> max;

        is >> ch; // read ']'
        if (ch != ']') {
            is.setstate(std::ios::failbit);
            return is;
        }

        r = Rect(min, max);
        return is;
    }


private:

    Coord m_min;
    Coord m_max;
};

} // end namespace mdn
