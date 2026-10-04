#pragma once

#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "CurlHttpClient.h"

namespace DekiHttp
{

/// Desktop SetupComponent that registers a CurlHttpClient with DekiHttp on
/// POSIX platforms. SetupComponent::RunEditorAutoSetups() runs it after
/// package load, along with WinHttpClientComponent; each installs its client
/// only on its own platform, so exactly one is used.
DEKI_CATEGORY("System")
DEKI_DISPLAY_NAME("curl HTTP Client")
DEKI_DESCRIPTION("Handles HTTP requests on Linux and macOS, for editor and desktop runs.")
DEKI_FORMER_NAME("CurlHttpClientComponent")
class CurlHttpClientComponent : public Deki::SetupComponent
{
public:
    CurlHttpClientComponent() = default;
    virtual ~CurlHttpClientComponent() = default;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "curl HTTP Client"; }
};

}  // namespace DekiHttp
