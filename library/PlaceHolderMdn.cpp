// library/PlaceHolderMdn.cpp
#include "PlaceHolderMdn.h"
#include <sstream>

PlaceHolderMdn::PlaceHolderMdn(int base) : base(base) {}

void PlaceHolderMdn::addValueAt(int x, int y, int value) {
    grid[{x, y}] += value;
    // Placeholder: implement carry-over logic later
}

int PlaceHolderMdn::getValueAt(int x, int y) const {
    auto it = grid.find({x, y});
    return (it != grid.end()) ? it->second : 0;
}

void PlaceHolderMdn::clear() {
    grid.clear();
}

std::string PlaceHolderMdn::toString() const {
    std::ostringstream oss;
    for (const auto& cell : grid) {
        oss << "(" << cell.first.first << "," << cell.first.second << ")=" << cell.second << " ";
    }
    return oss.str();
}
