#include "WinHttpClientComponent.h"
#include "DekiHttp.h"
#include <deki/LogSystem.h>

namespace DekiHttp
{

// A name of its own: package sources are compiled as a CMake unity build, so a
// file-static with the same name as another backend's would collide.
static WinHttpClient* s_WinHttpDriver = nullptr;

void WinHttpClientComponent::Setup(SetupCallback onComplete)
{
#ifdef _WIN32
    if (!s_WinHttpDriver)
    {
        s_WinHttpDriver = new WinHttpClient();
    }

    DekiHttp::SetCurrent(s_WinHttpDriver);
    DEKI_LOG_DEBUG("[deki-http] WinHttpClient registered with DekiHttp");
#else
    // No WinHTTP here. Leave the slot empty for CurlHttpClientComponent; a
    // registered stub would make the slot look filled while every request
    // fails.
#endif

    if (onComplete)
    {
        onComplete(true);
    }
}

// Runs when the project opens, not at Play. It only installs the client and
// makes no request, so it costs nothing, and it is ready before anything
// fetches (deki-gps waits for Play).
DEKI_REGISTER_EDITOR_AUTO_SETUP(WinHttpClientComponent);

}  // namespace DekiHttp
