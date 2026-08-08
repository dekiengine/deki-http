# Module descriptor for deki-engine auto-discovery
set(MODULE_DISPLAY_NAME "HTTP")
set(MODULE_PREFIX "DekiHttp")
set(MODULE_UPPER "HTTP")
set(MODULE_TARGET "deki-http")
set(MODULE_FILE_PREFIX "Http")
set(MODULE_ENTRY DekiHttpModule.cpp)

# WinHTTP: Windows-only system library, linked verbatim by DekiModule.cmake.
if(WIN32)
    set(MODULE_SYSTEM_LIBS winhttp)
endif()
