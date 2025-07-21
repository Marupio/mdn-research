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
    // Mdn2d slot0 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // slot0.setValue(COORD_ORIGIN, 3);
    // slot0.setValue(Coord(0, 1), 2);
    // slot0.setValue(Coord(1, 0), -2);
    // slot0.setValue(Coord(1, 1), 1);
    // slot0.setValue(Coord(20, 6), 9);

    // std::vector<std::string> slot0Disp = slot0.toStringRows();
    // for (auto riter = slot0Disp.rbegin(); riter != slot0Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    // Mdn2d slot1 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // slot1.add(COORD_ORIGIN, -4);
    // slot1.setValue(Coord(0, 1), 2);
    // slot1.setValue(Coord(1, 0), -2);
    // std::cout << "Prepare for carryover!" << std::endl;
    // slot1.carryover(COORD_ORIGIN);

    // // slot1.add(COORD_ORIGIN, 1000);
    // std::vector<std::string> slot1Disp = slot1.toStringRows();
    // for (auto riter = slot1Disp.rbegin(); riter != slot1Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    // Log_Info("Mdn2d slot2 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));");
    // Mdn2d slot2 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // Log_Info("slot2.add(COORD_ORIGIN, 3.141592635, Fraxis::X);");
    // slot2.add(COORD_ORIGIN, 3.141592635, Fraxis::X);
    // Log_Info("slot2.setValue(Coord(0, 1), 2);");
    // slot2.setValue(Coord(0, 1), 2);
    // Log_Info("slot2.setValue(Coord(1, 0), -2);");
    // slot2.setValue(Coord(1, 0), -2);
    // Log_Info("slot2.carryover(COORD_ORIGIN);");
    // slot2.carryover(COORD_ORIGIN);

    // // slot2.add(COORD_ORIGIN, 1000);
    // Log_Info("slot2 --> write");
    // std::vector<std::string> slot2Disp = slot2.toStringRows();
    // for (auto riter = slot2Disp.rbegin(); riter != slot2Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    // Log_Info("Mdn2d slot3 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));");
    // Mdn2d slot3 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // Log_Info("slot3.add(Coord(-1,0), 1.2, Fraxis::X);");
    // slot3.add(Coord(-1,0), 1.2, Fraxis::X);

    // // slot3.add(COORD_ORIGIN, 1000);
    // Log_Info("slot3 --> write");
    // std::vector<std::string> slot3Disp = slot3.toStringRows();
    // for (auto riter = slot3Disp.rbegin(); riter != slot3Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    // Log_Info("Mdn2d slot4 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));");
    // Mdn2d slot4 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // Log_Info("slot4.add(COORD_ORIGIN, 24601);");
    // slot4.add(COORD_ORIGIN, 1234);
    // Log_Info("slot4 += slot3;");
    // slot4 += slot3;

    // Log_Info("slot4 --> write");
    // std::vector<std::string> slot4Disp = slot4.toStringRows();
    // for (auto riter = slot4Disp.rbegin(); riter != slot4Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    // Mdn2d slot1 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // slot1.add(COORD_ORIGIN, -4);
    // slot1.setValue(Coord(0, 1), 2);
    // slot1.setValue(Coord(1, 0), -2);
    // std::cout << "Prepare for carryover!" << std::endl;
    // slot1.carryover(COORD_ORIGIN);

    // // slot1.add(COORD_ORIGIN, 1000);
    // std::vector<std::string> slot1Disp = slot1.toStringRows();
    // for (auto riter = slot1Disp.rbegin(); riter != slot1Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }

    Log_Info("NewInstance slot5");
    Mdn2d slot5 = Mdn2d::NewInstance(Mdn2dConfig(2, 32));
    Log_Info("slot5.add(COORD_ORIGIN, 10);");
    slot5.add(COORD_ORIGIN, 500);

    Log_Info("slot5 --> write");
    std::vector<std::string> slot5Disp = slot5.toStringRows();
    for (auto riter = slot5Disp.rbegin(); riter != slot5Disp.rend(); ++riter) {
        std::cout << *riter << '\n';
    }
    std::string boundsStr(Tools::pairToString(slot5.getBounds(), ","));
    Log_Info("slot5 bounds = " << slot5.getBounds().first << "," << slot5.getBounds().second);

    return 0;
}


int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // Switch Windows console to UTF-8
#endif
    std::cout <<  std::setprecision(19);
    std::cerr <<  std::setprecision(19);
    Logger& sirTalksALot = Logger::instance();
    sirTalksALot.setLevel(LogLevel::Debug4);
    sirTalksALot.setOutputToFile();

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
