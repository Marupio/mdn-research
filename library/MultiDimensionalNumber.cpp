// library/MultiDimensionalNumber.cpp
#include "MultiDimensionalNumber.h"
#include <sstream>

MultiDimensionalNumber::MultiDimensionalNumber(int base) : base(base) {}

void MultiDimensionalNumber::addValueAt(int x, int y, int value) {
    grid[{x, y}] += value;
    // Placeholder: implement carry-over logic later
}

int MultiDimensionalNumber::getValueAt(int x, int y) const {
    auto it = grid.find({x, y});
    return (it != grid.end()) ? it->second : 0;
}

void MultiDimensionalNumber::clear() {
    grid.clear();
}

std::string MultiDimensionalNumber::toString() const {
    std::ostringstream oss;
    for (const auto& cell : grid) {
        oss << "(" << cell.first.first << "," << cell.first.second << ")=" << cell.second << " ";
    }
    return oss.str();
}
