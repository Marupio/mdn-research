// Rect.hpp  — UPDATED
#pragma once

#include <vector>
#include <algorithm>
#include <limits>

#include "Coord.hpp"
#include "GlobalConfig.hpp"
#include "Logger.hpp"

// NEW: needed for iterators & sets
#include <iterator>
#include <unordered_set>
#include "CoordTypes.hpp" // CoordSet, VecCoord, VecVecCoord

namespace mdn {

class MDN_API Rect {

public:

    // *** Static member functions

    // Returns an 'invalid' Rect, meaning 'empty'
    static Rect GetInvalid() {
        return Rect(
            Coord(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
            Coord(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())
        );
    }

    // Return intersection of two rects. If they don't intersect, returns invalid (empty)
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


    // Position classification
    enum class FrontBack {
        NotApplicable = -1, // Rect is empty
        Behind = 0,
        BackEdge = 1,
        Inside = 2,
        FrontEdge = 3,
        InFront = 4
    };

    inline FrontBack HasCoordAt_X(const Coord& xy) const {
        if (isInvalid()) {
            return FrontBack::NotApplicable;
        }
        const int x = xy.x();
        int l = left();
        if (x < l) {
            return FrontBack::Behind;
        } else if (x == l) {
            return FrontBack::BackEdge;
        }
        int r = right();
        if (x < r) {
            return FrontBack::Inside;
        } else if (x == r) {
            return FrontBack::FrontEdge;
        }
        return FrontBack::InFront;
    }

    inline FrontBack HasCoordAt_Y(const Coord& xy) const {
        if (isInvalid()) {
            return FrontBack::NotApplicable;
        }
        const int y = xy.y();
        int b = bottom();
        if (y < b) {
            return FrontBack::Behind;
        } else if (y == b) {
            return FrontBack::BackEdge;
        }
        int r = right();
        if (y < r) {
            return FrontBack::Inside;
        } else if (y == r) {
            return FrontBack::FrontEdge;
        }
        return FrontBack::InFront;
    }


    enum class RelativePosition {
        NotApplicable, // Rect is empty
        Inside,
        OnWestEdge,
        OnEastEdge,
        OnNorthEdge,
        OnSouthEdge,
        OnNorthWestCorner,
        OnNorthEastCorner,
        OnSouthWestCorner,
        OnSouthEastCorner,
        East,
        NorthEast,
        North,
        NorthWest,
        West,
        SouthWest,
        South,
        SouthEast
    };
    //
    //        0  1  2  3  4 < fbX
    //  0  0  0  1  2  3  4
    //  1  5  5  6  7  8  9
    //  2 10 10 11 12 13 14
    //  3 15 15 16 17 18 19
    //  4 20 20 21 22 23 24
    //  ^  ^ fbY x 5
    //  fbY
    // Return the relative position of xy to this Rect
    inline RelativePosition HasCoordAt(const Coord& xy) const {
        if (isInvalid()) {
            return RelativePosition::NotApplicable;
        }

        FrontBack fbX = HasCoordAt_X(xy);
        FrontBack fbY = HasCoordAt_Y(xy);
        int xCode = static_cast<int>(fbX);
        int yCode = static_cast<int>(fbY);
        int code = xCode + yCode*5;
        switch(code) {
            case 0:
                return RelativePosition::NorthWest;
            case 1:
            case 2:
            case 3:
                return RelativePosition::North;
            case 4:
                return RelativePosition::NorthEast;
            case 5:
            case 10:
            case 15:
                return RelativePosition::West;
            case 6:
                return RelativePosition::OnNorthWestCorner;
            case 7:
                return RelativePosition::OnNorthEdge;
            case 8:
                return RelativePosition::OnNorthEastCorner;
            case 9:
            case 14:
            case 19:
                return RelativePosition::East;
            case 11:
                return RelativePosition::OnWestEdge;
            case 12:
                return RelativePosition::Inside;
            case 13:
                return RelativePosition::OnEastEdge;
            case 16:
                return RelativePosition::OnSouthWestCorner;
            case 17:
                return RelativePosition::OnSouthEdge;
            case 18:
                return RelativePosition::OnSouthEastCorner;
            case 20:
                return RelativePosition::SouthWest;
            case 21:
            case 22:
            case 23:
                return RelativePosition::South;
            case 24:
                return RelativePosition::SouthWest;
            default:
                Log_Warn("Invalid relative position");
                return RelativePosition::Inside;
        }
    }

    // *** Constructors

    // Construct null - retuns invalid (empty)
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

    // True if this rect has only a single coordinate
    bool isSingleCoord() const {
        return isValid() && m_min == m_max;
    }

    // True if this rect encompasses two or more coordinates
    bool isMultiCoord() const {
        return isValid() && m_min != m_max;
    }

    // True if c is inside or on edges
    bool contains(const Coord& c) const {
        return c.x() >= m_min.x() && c.x() <= m_max.x()
            && c.y() >= m_min.y() && c.y() <= m_max.y();
    }

    void clear() {
        *this = Rect::GetInvalid();
    }

    void set(Coord xy) {
        m_min = xy;
        m_max = xy;
    }
    void set(int x, int y) {
        m_min = Coord(x, y);
        m_max = m_min;
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
        coords.reserve(size());
        for (int y = bottom(); y <= top(); ++y) {
            for (int x = left(); x <= right(); ++x) {
                coords.emplace_back(x, y);
            }
        }
        return coords;
    }

    // Row-major iterator support (y from bottom..top, x from left..right)
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = Coord;
        using difference_type   = std::ptrdiff_t;
        using reference         = Coord;   // returns by value (immutable w.r.t Rect)
        using pointer           = void;

        const_iterator() : r(nullptr), x(0), y(0) {}
        const_iterator(const Rect* rect, int xi, int yi) : r(rect), x(xi), y(yi) {}

        reference operator*() const { return Coord{x, y}; }

        const_iterator& operator++() {
            if (!r) return *this;
            ++x;
            if (x > r->right()) { x = r->left(); ++y; }
            return *this;
        }
        const_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const const_iterator& o) const { return r == o.r && x == o.x && y == o.y; }
        bool operator!=(const const_iterator& o) const { return !(*this == o); }

    private:
        const Rect* r;
        int x, y;
    };

    // Provide begin/end. Note: iteration yields Coord by value.
    const_iterator begin() const {
        if (!isValid()) return end();
        return const_iterator(this, left(), bottom());
    }
    const_iterator end() const {
        // end() is "one past last row"
        return const_iterator(this, left(), top() + 1);
    }
    // Optional alias so `for (auto c : rect)` works without constness fuss.
    using iterator = const_iterator;

    // Return entire area as rows (index = y - bottom(), each row left..right)
    [[nodiscard]] VecVecCoord asRows() const {
        VecVecCoord result;
        if (!isValid()) return result;
        const int W = width();
        const int H = height();
        result.resize(static_cast<size_t>(H));
        for (int row = 0; row < H; ++row) {
            result[row].reserve(static_cast<size_t>(W));
            const int y = bottom() + row;
            for (int x = left(); x <= right(); ++x) {
                result[row].emplace_back(x, y);
            }
        }
        return result;
    }

    // Return entire area as columns (index = x - left(), each column bottom..top)
    [[nodiscard]] VecVecCoord asColumns() const {
        VecVecCoord result;
        if (!isValid()) return result;
        const int W = width();
        const int H = height();
        result.resize(static_cast<size_t>(W));
        for (int col = 0; col < W; ++col) {
            result[col].reserve(static_cast<size_t>(H));
            const int x = left() + col;
            for (int y = bottom(); y <= top(); ++y) {
                result[col].emplace_back(x, y);
            }
        }
        return result;
    }

    // NEW: return a set containing all coords inside the rect
    [[nodiscard]] CoordSet asCoordSet() const {
        CoordSet out;
        if (!isValid()) return out;
        out.reserve(static_cast<size_t>(size()));
        for (int y = bottom(); y <= top(); ++y) {
            for (int x = left(); x <= right(); ++x) {
                out.emplace(x, y);
            }
        }
        return out;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        if (!r.isValid()) {
            os << "[Empty]";
        } else {
            os << "[" << r.min() << "->" << r.max() << "]";
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
