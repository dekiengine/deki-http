#include "CurlHttpClientComponent.h"
#include "DekiHttp.h"
#include <deki/LogSystem.h>

namespace DekiHttp
{

// Distinct name per translation unit on purpose: package sources are compiled as
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

// Project open, not Play. This only installs the client object; it makes no
// request of its own, so it costs nothing to have ready. It also has to be
// up before anything that fetches — deki-gps waits for Play, which is after
// this either way.
DEKI_REGISTER_EDITOR_AUTO_SETUP(CurlHttpClientComponent);

}  // namespace DekiHttp
