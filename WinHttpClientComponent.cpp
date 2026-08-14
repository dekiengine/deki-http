#include "WinHttpClientComponent.h"
#include "DekiHttp.h"
#include "DekiLogSystem.h"

// Distinct name per translation unit on purpose: module sources are compiled as
// a CMake unity build, so a file-static shared with another backend's component
// would collide at the merged-TU level.
static WinHttpClient* s_WinHttpDriver = nullptr;

void WinHttpClientComponent::Setup(SetupCallback onComplete)
{
#ifdef _WIN32
    if (!s_WinHttpDriver)
        s_WinHttpDriver = new WinHttpClient();

    DekiHttp::SetCurrent(s_WinHttpDriver);
    DEKI_LOG_INFO("[deki-http] WinHttpClient registered with DekiHttp");
#else
    // WinHTTP does not exist here. Previously this registered anyway, so every
    // request reached a stub that logged "called on non-Windows platform" and
    // returned empty — the driver slot looked filled while nothing worked.
    // Leave it empty for CurlHttpClientComponent to claim.
#endif

    if (onComplete) onComplete(true);
}

DEKI_REGISTER_EDITOR_AUTO_SETUP(WinHttpClientComponent);
