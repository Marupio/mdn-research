// library/PlaceHolderMdn.cpp
#include "PlaceHolderMdn.h"
#include <sstream>

mdn::PlaceHolderMdn::PlaceHolderMdn(int base) : base(base) {}

void mdn::PlaceHolderMdn::addValueAt(int x, int y, int value) {
    grid[{x, y}] += value;
    // Placeholder: implement carry-over logic later
}

int mdn::PlaceHolderMdn::getValueAt(int x, int y) const {
    auto it = grid.find({x, y});
    return (it != grid.end()) ? it->second : 0;
}

void mdn::PlaceHolderMdn::clear() {
    grid.clear();
}

std::string mdn::PlaceHolderMdn::toString() const {
    std::ostringstream oss;
    for (const auto& cell : grid) {
        oss << "(" << cell.first.first << "," << cell.first.second << ")=" << cell.second << " ";
    }
    return oss.str();
}
