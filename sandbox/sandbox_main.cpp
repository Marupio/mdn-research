#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>


// MDN headers
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

int main() {
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
