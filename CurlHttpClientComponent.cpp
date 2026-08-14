#include "CurlHttpClientComponent.h"
#include "DekiHttp.h"
#include "DekiLogSystem.h"

// Distinct name per translation unit on purpose: module sources are compiled as
// a CMake unity build, so a file-static shared with another backend's component
// would collide at the merged-TU level.
static CurlHttpClient* s_CurlHttpDriver = nullptr;

void CurlHttpClientComponent::Setup(SetupCallback onComplete)
{
#ifdef _WIN32
    // Windows is served by WinHttpClientComponent. Registering here too would
    // make the winner depend on auto-setup ordering.
    if (onComplete) onComplete(true);
#else
    if (!s_CurlHttpDriver)
        s_CurlHttpDriver = new CurlHttpClient();

    DekiHttp::SetCurrent(s_CurlHttpDriver);
    DEKI_LOG_INFO("[deki-http] CurlHttpClient registered with DekiHttp");

    if (onComplete) onComplete(true);
#endif
}

DEKI_REGISTER_EDITOR_AUTO_SETUP(CurlHttpClientComponent);
