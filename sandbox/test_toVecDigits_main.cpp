#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>


// MDN headers
#include "Digit.hpp"
#include "Logger.hpp"
#include "Mdn2d.hpp"
#include "Mdn2dBase.hpp"
#include "Mdn2dConfig.hpp"
#include "Mdn2dIO.hpp"
#include "Rect.hpp"
#include "Digit.hpp"
#include "Tools.hpp"
#include "NamedEnum.hpp"


namespace mdn {
#define COLOUR_LIST(Y) Y(unknown) Y(red) Y(green) Y(blue)
DECLARE_NAMED_ENUM(Colour, COLOUR_LIST, unknown, CaseMode::Insensitive, WarnWithLogger<false>)
};
// using namespace mdn;

int testEnums() {
    mdn::Colour c = mdn::Colour::red;
    Log_Info("The colour is " << to_string(c) << ", yay!");

    std::string s = "BLUE";
    mdn::Colour cs = mdn::parse_Colour(s);
    Log_Info("The colour is " << cs << ", yay!");
    switch (cs) {
        case mdn::Colour::red:
            Log_Info("red");
            break;
        case mdn::Colour::green:
            Log_Info("green");
            break;
        case mdn::Colour::blue:
            Log_Info("blue");
            break;
    }

    return 0;
}

int main() {
    std::string instr;
    bool keepGoing = true;
    while (keepGoing) {
        int base = -1;
        while (base < 0) {
            std::cout << "\nBase ? ";
            std::getline(std::cin, instr);
            if (instr == "quit" || instr == "exit") {
                return 0;
            }
            int tmp = std::atoi(instr.c_str());
            if (tmp >= 2 && tmp <= 32) {
                base = tmp;
            }
        }
        std::cout << "Number ? ";
        std::getline(std::cin, instr);
        if (instr == "quit" || instr == "exit") {
            return 0;
        }
        bool negative = false;
        long double val = std::stold(instr);

        // Outputs
        mdn::VecDigit digits;
        int offset;
        std::pair<int, int> kRange;

        bool success(
            mdn::Tools::toVecDigits(
                val,
                base,
                digits,
                offset,
                kRange
            )
        );

        if (!success) {
            std::cout << "\nFailure!\n" << std::endl;
        } else {
            std::cout << "\nSuccess!\n" << std::endl;
            for (mdn::Digit d : digits) {
                std::cout << int(d) << " ";
            }
            std::cout << "\noffset = " << offset << std::endl;
            std::cout << "kRange = (" << kRange.first << "," << kRange.second << ")" << std::endl;
        }
    }
    return 0;
}
