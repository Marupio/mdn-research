#pragma once

#include <utility>
#include <functional>

#include <utility>

namespace mdn {
using Coord = ::std::pair<int, int>;
}; // end namespace mdn


namespace std {
    template <>
    struct hash<mdn::Coord> {
        std::size_t operator()(const mdn::Coord& coord) const noexcept {
            std::size_t h1 = std::hash<int>{}(coord.first);
            std::size_t h2 = std::hash<int>{}(coord.second);
            return h1 ^ (h2 << 1); // or use boost::hash_combine if needed
        }
    };
}
