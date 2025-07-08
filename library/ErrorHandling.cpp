#include <cxxabi.h>
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>

#include "ErrorHandling.h"

void mdn::ErrorHandling::PrintStackTrace1() {
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    std::cerr << "Remangled Stack trace:\n";
    for (int i = 0; i < frames; ++i) {
        std::string cmd = "addr2line -e sandbox/sandbox -f -C -p ";
        std::string parseme(strs[i]);
        cmd += std::to_string(reinterpret_cast<uintptr_t>(callstack[i]));
        std::cerr << "parseme = [" << parseme << "]" << '\n';
        std::system(cmd.c_str());
        std::cerr << "cmd = " << cmd.c_str() << '\n';
        std::cerr << "calling demangled\n";
        std::string dm(Demangle(strs[i]));
        std::cerr << "dm = " << dm << '\n';
    }
    free(strs);
}


bool splitEnclosed(const std::string& input, std::string& outA, std::string& outB) {
    auto open = input.find('(');
    auto close = input.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close <= open) {
        return false; // malformed
    }

    outA = input.substr(0, open);
    outB = input.substr(open + 1, close - open - 1);
    return true;
}


std::string mdn::ErrorHandling::Demangle(const char* name) {
    int status = -1;
    std::cerr << "name before = " << name << '\n';
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result = (status == 0) ? demangled : name;
    // std::cerr << "demangled = " << demangled << "\n";
    std::cerr << "name = [" << name << "]\n";
    std::cerr << "status = " << status << "\n";
    free(demangled);
    return result;
}


void mdn::ErrorHandling::PrintStackTrace() {
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    std::cerr << "Remangled Stack trace:\n";
    for (int i = 0; i < frames; ++i) {
        std::string parseme(strs[i]);
        std::string outA, outB;
        if (!splitEnclosed(parseme, outA, outB)) {
            std::cerr << "FAILED TO PARSE: " << parseme << '\n';
            continue;
        }
        std::string cmd = "addr2line -e " + outA + " -f -C -p " + outB;
        std::system(cmd.c_str());
        // std::cerr << "cmd = " << cmd.c_str() << '\n';
        // std::cerr << "calling demangled, result = " << Demangle(strs[i]) << '\n';
    }
    free(strs);
}
