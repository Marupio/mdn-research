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

