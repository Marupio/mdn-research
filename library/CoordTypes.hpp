#pragma once

#include <unordered_set>
#include <vector>

#include "Coord.hpp"

namespace mdn {

using CoordSet = std::unordered_set<Coord>;
using VecCoord = std::vector<Coord>;
using VecVecCoord = std::vector<VecCoord>;

} // end namespace mdn