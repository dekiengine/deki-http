#include "WinHttpClientComponent.h"
#include "DekiHttp.h"
#include "DekiLogSystem.h"

static WinHttpClient* s_Driver = nullptr;

void WinHttpClientComponent::Setup(SetupCallback onComplete)
{
    if (!s_Driver)
        s_Driver = new WinHttpClient();

    DekiHttp::SetCurrent(s_Driver);
    DEKI_LOG_INFO("[deki-http] WinHttpClient registered with DekiHttp");

    if (onComplete) onComplete(true);
}

DEKI_REGISTER_EDITOR_AUTO_SETUP(WinHttpClientComponent);
