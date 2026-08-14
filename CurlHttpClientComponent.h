#pragma once

#include "SetupComponent.h"
#include "reflection/DekiProperty.h"
#include "CurlHttpClient.h"

/**
 * @brief Desktop SetupComponent that registers a CurlHttpClient with DekiHttp
 * on POSIX platforms. Auto-fired by SetupComponent::RunEditorAutoSetups()
 * after module load, alongside WinHttpClientComponent — each installs its
 * driver only on the platform it supports, so exactly one wins.
 */
class CurlHttpClientComponent : public SetupComponent
{
public:
    DEKI_COMPONENT(CurlHttpClientComponent, SetupComponent, "System", "5c1f9a47-8e2b-4d73-b6a0-9d3c47f1e820", "")
    DEKI_DISPLAY_NAME("curl HTTP Client")
    DEKI_DESCRIPTION("Handles HTTP requests on Linux and macOS, for editor and desktop runs.")

    CurlHttpClientComponent() = default;
    virtual ~CurlHttpClientComponent() = default;

    void        Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "curl HTTP Client"; }
};

#include "generated/CurlHttpClientComponent.gen.h"
