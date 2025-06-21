// library/MultiDimensionalNumber.h
#ifndef PLACEHOLDERMDN_H
#define PLACEHOLDERMDN_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

class PlaceHolderMdn {
public:
    PlaceHolderMdn(int base = 10);

    void addValueAt(int x, int y, int value);
    int getValueAt(int x, int y) const;
    std::string toString() const;
    void clear();

private:
    int base;
    std::map<std::pair<int, int>, int> grid;
};

#endif // PLACEHOLDERMDN_H
