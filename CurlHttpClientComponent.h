#pragma once

#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>
#include "CurlHttpClient.h"

/**
 * @brief Desktop SetupComponent that registers a CurlHttpClient with DekiHttp
 * on POSIX platforms. Auto-fired by SetupComponent::RunEditorAutoSetups()
 * after package load, alongside WinHttpClientComponent — each installs its
 * driver only on the platform it supports, so exactly one wins.
 */
DEKI_CATEGORY("System")
DEKI_DISPLAY_NAME("curl HTTP Client")
DEKI_DESCRIPTION("Handles HTTP requests on Linux and macOS, for editor and desktop runs.")
class CurlHttpClientComponent : public Deki::SetupComponent
{
public:

    CurlHttpClientComponent() = default;
    virtual ~CurlHttpClientComponent() = default;

    void        Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "curl HTTP Client"; }
};

