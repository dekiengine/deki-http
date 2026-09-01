#include "DekiHttpPackage.h"
#include "interop/DekiPlugin.h"
#include "DekiLogSystem.h"
#include "DekiHttp.h"

#ifdef DEKI_EDITOR

extern void DekiHttp_RegisterComponents();
extern int  DekiHttp_GetAutoComponentCount();
extern const DekiComponentMeta* DekiHttp_GetAutoComponentMeta(int index);

static bool s_HttpRegistered = false;

extern "C" {

DEKI_HTTP_API int DekiHttp_EnsureRegistered(void)
{
    if (s_HttpRegistered)
        return DekiHttp_GetAutoComponentCount();
    s_HttpRegistered = true;
    DekiHttp_RegisterComponents();
    return DekiHttp_GetAutoComponentCount();
}

DEKI_PLUGIN_API const char* DekiPlugin_GetName(void)    { return "Deki HTTP Package"; }
DEKI_PLUGIN_API const char* DekiPlugin_GetVersion(void)
{
#ifdef DEKI_PACKAGE_VERSION
    return DEKI_PACKAGE_VERSION;
#else
    return "0.0.0-dev";
#endif
}
DEKI_PLUGIN_API int  DekiPlugin_Init(void)             { DEKI_LOG_INFO("[deki-http] DekiPlugin_Init"); return 0; }
DEKI_PLUGIN_API void DekiPlugin_Shutdown(void)
{
    s_HttpRegistered = false;
    DekiHttp::SetCurrent(nullptr);
}
DEKI_PLUGIN_API int  DekiPlugin_GetComponentCount(void){ return DekiHttp_GetAutoComponentCount(); }
DEKI_PLUGIN_API const DekiComponentMeta* DekiPlugin_GetComponentMeta(int index)
{
    return DekiHttp_GetAutoComponentMeta(index);
}
DEKI_PLUGIN_API void DekiPlugin_RegisterComponents(void)
{
    int n = DekiHttp_EnsureRegistered();
    DEKI_LOG_INFO("[deki-http] DekiPlugin_RegisterComponents -> %d component(s)", n);
}


} // extern "C"

#endif // DEKI_EDITOR
