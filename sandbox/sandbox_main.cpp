#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>

#include "ErrorHandling.h"
#include "Mdn2d.h"

using namespace std;
using namespace mdn;

int execute() {
    Mdn2d slot0 = Mdn2d::NewInstance();
    slot0.setValue(COORD_ORIGIN, 3);
    slot0.setValue(Coord(0, 1), 2);
    slot0.setValue(Coord(1, 0), -2);
    slot0.setValue(Coord(1, 1), 1);
    std::vector<std::string> disp0 = slot0.toStringRows();
    for (auto riter = disp0.rbegin(); riter != disp0.rend(); ++riter) {
        std::cout << *riter << '\n';
    }
    return 0;
}


int main() {
    int returnValue = -1;
    try {
        returnValue = execute();
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << '\n';
        ErrorHandling::PrintStackTrace();
            std::exit(EXIT_FAILURE);
    }
    return returnValue;
}
