#pragma once

#include <string>

#include "GlobalConfig.h"

namespace mdn {

class MDN_API ErrorHandling {

public:

    static void PrintStackTrace();
    static void PrintStackTrace1();

    static std::string Demangle(const char* name);

};

} // end namespace mdn