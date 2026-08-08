#pragma once

#include "SetupComponent.h"
#include "reflection/DekiProperty.h"
#include "WinHttpClient.h"

/**
 * @brief Editor / desktop SetupComponent that registers a WinHttpClient with
 * DekiHttp. Auto-fired by SetupComponent::RunEditorAutoSetups() after
 * module load.
 */
class WinHttpClientComponent : public SetupComponent
{
public:
    DEKI_COMPONENT(WinHttpClientComponent, SetupComponent, "System", "8b3a7f12-4d6e-4c95-a814-2f9e0d5b8c3a", "")
    DEKI_DISPLAY_NAME("WinHTTP Client")
    DEKI_DESCRIPTION("Handles HTTP requests on Windows, for editor and desktop runs.")

    WinHttpClientComponent() = default;
    virtual ~WinHttpClientComponent() = default;

    void        Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "WinHTTP Client"; }
};

#include "generated/WinHttpClientComponent.gen.h"
