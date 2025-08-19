#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <tuple>

#include "GlobalConfig.hpp"

namespace mdn {

class MDN_API Coord {
public:
    constexpr Coord(int x = 0, int y = 0) : m_x(x), m_y(y) {}

    // Element accessors
    int x() const { return m_x; }
    int& x() { return m_x; }
    int y() const { return m_y; }
    int& y() { return m_y; }

    void set(int x, int y) {
        m_x = x;
        m_y = y;
    }

    // *** Functions that create a new Coord instance

        // Creates a new Coord, translated by the given (xIncr, yIncr)
        Coord translated(int xIncr, int yIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            ret.m_y += yIncr;
            return ret;
        }
        // Creates a new Coord, translated along x by the given xIncr
        Coord translatedX(int xIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            return ret;
        }
        // Creates a new Coord, translated along y by the given yIncr
        Coord translatedY(int yIncr) const {
            Coord ret(*this);
            ret.m_y += yIncr;
            return ret;
        }


    // *** Functions that modify *this

        // Move the Coord by the given (xIncr, yIncr)
        void translate(int xIncr, int yIncr) { m_x += xIncr; m_y += yIncr; }
        // Move the Coord along x by the given xIncr
        void translateX(int xIncr) { m_x += xIncr; }
        // Move the Coord along y by the given yIncr
        void translateY(int yIncr) { m_y += yIncr; }

    // Compound operators
    Coord& operator+=(int val) {
        m_x += val;
        m_y += val;
        return *this;
    }

    Coord& operator*=(int val) {
        m_x *= val;
        m_y *= val;
        return *this;
    }

    Coord& operator/=(int val) {
        m_x /= val;
        m_y /= val;
        return *this;
    }

    Coord& operator-=(int val) {
        m_x -= val;
        m_y -= val;
        return *this;
    }

    Coord& operator+=(const Coord& other) {
        m_x += other.m_x;
        m_y += other.m_y;
        return *this;
    }

    Coord& operator-=(const Coord& other) {
        m_x -= other.m_x;
        m_y -= other.m_y;
        return *this;
    }

    // Arithmetic operators
    Coord operator+(int val) const {
        return Coord(*this) += val;
    }

    Coord operator-(int val) const {
        return Coord(*this) -= val;
    }

    Coord operator*(int val) const {
        return Coord(*this) *= val;
    }

    Coord operator/(int val) const {
        return Coord(*this) /= val;
    }

    Coord operator+(const Coord& other) const {
        return Coord(*this) += other;
    }

    Coord operator-(const Coord& other) const {
        return Coord(*this) -= other;
    }

    bool operator==(const Coord& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }

    bool operator!=(const Coord& other) const {
        return !(*this == other);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Coord& c) {
        return os << "(" << c.m_x << "," << c.m_y << ")";
    }

    friend std::istream& operator>>(std::istream& is, Coord& c) {
        char lparen, comma, rparen;
        return (is >> lparen >> c.m_x >> comma >> c.m_y >> rparen);
    }

    // Structured binding support
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 0>) noexcept {
        return c.m_x;
    }
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 1>) noexcept {
        return c.m_y;
    }


private:
    int m_x;
    int m_y;
};

constexpr Coord COORD_ORIGIN = Coord{0, 0};

} // namespace mdn

namespace std {
    template <>
    struct hash<mdn::Coord> {
        std::size_t operator()(const mdn::Coord& c) const noexcept {
            std::size_t h1 = std::hash<int>{}(c.x());
            std::size_t h2 = std::hash<int>{}(c.y());
            return h1 ^ (h2 << 1);
        }
    };

    template <>
    struct tuple_size<mdn::Coord> : std::integral_constant<std::size_t, 2> {};

    template <std::size_t N>
    struct tuple_element<N, mdn::Coord> {
        using type = int;
    };

    template <std::size_t N>
    constexpr int get(const mdn::Coord& c) noexcept {
        static_assert(N < 2, "Index out of range for mdn::Coord");
        if constexpr (N == 0) return c.x();
        else return c.y();
    }
}
