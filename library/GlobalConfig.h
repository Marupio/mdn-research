#pragma once

#ifdef _WIN32
  #ifdef mdn_EXPORTS
    #define MDN_API __declspec(dllexport)
  #else
    #define MDN_API __declspec(dllimport)
  #endif
#else
  #define MDN_API
#endif


constexpr const char* past_last_slash(const char* path) {
    const char* last = path;
    while (*path) {
        if (*path == '/' || *path == '\\') last = path + 1;
        ++path;
    }
    return last;
}

#define SHORT_FILE (past_last_slash(__FILE__))

// #define SHORT_FILE ({constexpr cstr sf__ {past_last_slash(__FILE__)}; sf__;})
// extern const char* PathToPrettyName(const char*);
// static const char* TranslationUnitName(const char* Path)
// {
// static const char* Name=PathToPrettyName(Path);
// return Name;
// }
// #define FILENAME__ TranslationUnitName(__FILE__)