#pragma once

#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "WinHttpClient.h"

namespace DekiHttp
{

/// Editor and desktop SetupComponent that registers a WinHttpClient with
/// DekiHttp on Windows. SetupComponent::RunEditorAutoSetups() runs it after
/// package load.
DEKI_CATEGORY("System")
DEKI_DISPLAY_NAME("WinHTTP Client")
DEKI_DESCRIPTION("Handles HTTP requests on Windows, for editor and desktop runs.")
class WinHttpClientComponent : public Deki::SetupComponent
{
public:
    WinHttpClientComponent() = default;
    virtual ~WinHttpClientComponent() = default;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "WinHTTP Client"; }
};

}  // namespace DekiHttp
