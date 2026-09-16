#pragma once

#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "WinHttpClient.h"

namespace DekiHttp
{

/**
 * @brief Editor / desktop SetupComponent that registers a WinHttpClient with
 * DekiHttp. Auto-fired by SetupComponent::RunEditorAutoSetups() after
 * package load.
 */
DEKI_CATEGORY("System")
DEKI_DISPLAY_NAME("WinHTTP Client")
DEKI_DESCRIPTION("Handles HTTP requests on Windows, for editor and desktop runs.")
DEKI_FORMER_NAME("WinHttpClientComponent")
class WinHttpClientComponent : public Deki::SetupComponent
{
public:

    WinHttpClientComponent() = default;
    virtual ~WinHttpClientComponent() = default;

    void        Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "WinHTTP Client"; }
};

}  // namespace DekiHttp

