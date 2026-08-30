# Package descriptor for deki-engine auto-discovery
set(PACKAGE_DISPLAY_NAME "HTTP")
set(PACKAGE_PREFIX "DekiHttp")
set(PACKAGE_UPPER "HTTP")
set(PACKAGE_TARGET "deki-http")
set(PACKAGE_FILE_PREFIX "Http")
set(PACKAGE_ENTRY DekiHttpPackage.cpp)

# WinHTTP: Windows-only system library, linked verbatim by DekiPackage.cmake.
if(WIN32)
    set(PACKAGE_SYSTEM_LIBS winhttp)
endif()
