#include "Mdn2d.h"
#include <stdexcept>
#include <sstream>

mdn::Mdn2d::Mdn2d(int base) : m_base(base) {}


void mdn::Mdn2d::addValueAt(int x, int y, int value) {
    m_raw[{x, y}] += static_cast<Digit>(value);
}


void mdn::Mdn2d::addValueAt(Coord xy, int value) {
    m_raw[xy] += static_cast<Digit>(value);
}


mdn::Digit mdn::Mdn2d::getValueAt(int x, int y) const {
    auto it = m_raw.find({x, y});
    return it != m_raw.end() ? it->second : 0;
}


mdn::Digit mdn::Mdn2d::getValueAt(Coord xy) const {
    auto it = m_raw.find(xy);
    return it != m_raw.end() ? it->second : 0;
}


void mdn::Mdn2d::clear() {
    m_raw.clear();
}


std::string mdn::Mdn2d::toString() const {
    std::ostringstream oss;
    for (const auto& [coord, val] : m_raw) {
        oss << "(" << coord.first << "," << coord.second << ")=" << static_cast<int>(val) << "\n";
    }
    return oss.str();
}


void mdn::Mdn2d::purgeZeroes() {
    for (auto it = m_raw.begin(); it != m_raw.end(); ) {
        if (it->second == 0) {
            it = m_raw.erase(it);
        } else {
            ++it;
        }
    }
}


mdn::Digit& mdn::Mdn2d::operator()(int x, int y) {
    return operator()(Coord(x, y));
}


mdn::Digit& mdn::Mdn2d::operator()(Coord xy) {
    return m_raw[xy];
}


mdn::Digit mdn::Mdn2d::operator()(int x, int y) const {
    return operator()(Coord(x, y));
}


mdn::Digit mdn::Mdn2d::operator()(Coord xy) const {
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        return Digit(0);
    }
    return it->second;
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    for (const auto& [coord, val] : rhs.m_raw) {
        m_raw[coord] += val;
    }
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    for (const auto& [coord, val] : rhs.m_raw) {
        m_raw[coord] -= val;
    }
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    // Stub: implement proper MDN multiplication logic
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    // Stub: implement proper MDN division logic
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    for (auto& [_, val] : m_raw) {
        val *= scalar;
    }
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(int scalar) {
    for (auto& [_, val] : m_raw) {
        val /= scalar;
    }
    return *this;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    return m_raw == rhs.m_raw;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    return !(*this == rhs);
}
