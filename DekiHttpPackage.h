#pragma once

// DLL export macro
#ifdef _WIN32
    #ifdef DEKI_HTTP_EXPORTS
        #define DEKI_HTTP_API __declspec(dllexport)
    #else
        #define DEKI_HTTP_API __declspec(dllimport)
    #endif
#else
    #define DEKI_HTTP_API __attribute__((visibility("default")))
#endif
