// library/MultiDimensionalNumber.h
#ifndef MULTIDIMENSIONALNUMBER_H
#define MULTIDIMENSIONALNUMBER_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

class MultiDimensionalNumber {
public:
    MultiDimensionalNumber(int base = 10);

    void addValueAt(int x, int y, int value);
    int getValueAt(int x, int y) const;
    std::string toString() const;
    void clear();

private:
    int base;
    std::map<std::pair<int, int>, int> grid;
};

#endif // MULTIDIMENSIONALNUMBER_H
