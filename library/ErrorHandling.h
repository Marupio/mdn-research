#pragma once

#include <string>

namespace mdn {

class ErrorHandling {

public:

    static void PrintStackTrace();
    static void PrintStackTrace1();

    static std::string Demangle(const char* name);

};

} // end namespace mdn