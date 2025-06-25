#pragma once

#include <utility>
#include <functional>
#include <iostream>

namespace mdn {

using Coord = ::std::pair<int, int>;
const Coord COORD_ORIGIN = Coord({0, 0});

}; // end namespace mdn


// Stream operator
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& pair) {
    return os << "(" << pair.first << ", " << pair.second << ")";
}

// Conversion to string
template <typename T1, typename T2>
std::string to_string(const std::pair<T1, T2>& pair) {
    std::ostringstream oss;
    oss << pair;
    return oss.str();
}



namespace std {
    // Hashing function
    template <>
    struct hash<mdn::Coord> {
        std::size_t operator()(const mdn::Coord& coord) const noexcept {
            std::size_t h1 = std::hash<int>{}(coord.first);
            std::size_t h2 = std::hash<int>{}(coord.second);
            return h1 ^ (h2 << 1); // or use boost::hash_combine if needed
        }
    };
}
