/**
 * @file DekiHttpPackage.cpp
 * @brief Package entry point for deki-http
 */
#include "DekiHttpPackage.h"
#include <deki/interop/Plugin.h>
#include <deki/LogSystem.h>
#include "DekiHttp.h"

extern void DekiHttpRegisterComponents();
extern int DekiHttpGetAutoComponentCount();
extern const Deki::ComponentMeta* DekiHttpGetAutoComponentMeta(int index);

namespace DekiHttp
{

#ifdef DEKI_EDITOR

static bool s_HttpRegistered = false;

// The exports below are C symbols at global scope; the package's own
// registration helpers and statics live in its namespace.
using namespace DekiHttp;

extern "C"
{
    DEKI_HTTP_API int DekiHttpEnsureRegistered(void)
    {
        if (s_HttpRegistered)
        {
            return ::DekiHttpGetAutoComponentCount();
        }
        s_HttpRegistered = true;
        ::DekiHttpRegisterComponents();
        return ::DekiHttpGetAutoComponentCount();
    }

    DEKI_PLUGIN_API const char* DekiPluginGetName(void)
    {
        return "Deki HTTP Package";
    }
    DEKI_PLUGIN_API const char* DekiPluginGetVersion(void)
    {
#ifdef DEKI_PACKAGE_VERSION
        return DEKI_PACKAGE_VERSION;
#else
        return "0.0.0-dev";
#endif
    }
    DEKI_PLUGIN_API int DekiPluginInit(void)
    {
        return 0;
    }
    DEKI_PLUGIN_API void DekiPluginShutdown(void)
    {
        s_HttpRegistered = false;
        DekiHttp::SetCurrent(nullptr);
    }
    DEKI_PLUGIN_API int DekiPluginGetComponentCount(void)
    {
        return ::DekiHttpGetAutoComponentCount();
    }
    DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPluginGetComponentMeta(int index)
    {
        return ::DekiHttpGetAutoComponentMeta(index);
    }
    DEKI_PLUGIN_API void DekiPluginRegisterComponents(void)
    {
        DekiHttpEnsureRegistered();
    }

}  // extern "C"

#endif  // DEKI_EDITOR
}  // namespace DekiHttp
