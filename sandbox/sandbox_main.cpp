#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>

#include "Logger.h"
#include "Mdn2d.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace mdn;

int execute() {
    Mdn2d slot0 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    slot0.setValue(COORD_ORIGIN, 3);
    slot0.setValue(Coord(0, 1), 2);
    slot0.setValue(Coord(1, 0), -2);
    slot0.setValue(Coord(1, 1), 1);
    slot0.setValue(Coord(20, 6), 9);
    std::vector<std::string> disp0 = slot0.toStringRows();
    for (auto riter = disp0.rbegin(); riter != disp0.rend(); ++riter) {
        std::cout << *riter << '\n';
    }
    return 0;
}


int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // Switch Windows console to UTF-8
#endif
    Logger::instance().setLevel(LogLevel::Debug3);

    int returnValue = -1;
    try {
        returnValue = execute();
    } catch (const std::exception& ex) {
        std::ostringstream oss;
        oss << "Unhandled exception: " << ex.what() << '\n';
        Logger::instance().error(oss.str());
        std::cerr << oss.str() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return returnValue;
}
