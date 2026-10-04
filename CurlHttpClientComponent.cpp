#include "CurlHttpClientComponent.h"
#include "DekiHttp.h"
#include <deki/LogSystem.h>

namespace DekiHttp
{

// A name of its own: package sources are compiled as a CMake unity build, so a
// file-static with the same name as another backend's would collide.
static CurlHttpClient* s_CurlHttpDriver = nullptr;

void CurlHttpClientComponent::Setup(SetupCallback onComplete)
{
#ifdef _WIN32
    // Windows uses WinHttpClientComponent. Registering here too would make
    // the result depend on the order auto-setups run in.
    if (onComplete)
    {
        onComplete(true);
    }
#else
    if (!s_CurlHttpDriver)
    {
        s_CurlHttpDriver = new CurlHttpClient();
    }

    DekiHttp::SetCurrent(s_CurlHttpDriver);
    DEKI_LOG_DEBUG("[deki-http] CurlHttpClient registered with DekiHttp");

    if (onComplete)
    {
        onComplete(true);
    }
#endif
}

// Runs when the project opens, not at Play. It only installs the client and
// makes no request, so it costs nothing, and it is ready before anything
// fetches (deki-gps waits for Play).
DEKI_REGISTER_EDITOR_AUTO_SETUP(CurlHttpClientComponent);

}  // namespace DekiHttp
