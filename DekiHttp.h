#pragma once

#include "IDekiHttpClient.h"
#include "DekiHttpModule.h"

/**
 * @brief Active-driver registry and facade for HTTP.
 *
 * A platform integration module registers its IDekiHttpClient implementation
 * via SetCurrent() during its setup phase. Consumers call DekiHttp::FetchUrl
 * / Get / PostJson directly — the singleton is dllexported from deki-http.dll
 * and imported by every consumer module.
 *
 * Lives in deki-http.dll (moved out of deki-engine-core) so that engine-core
 * stays free of concrete service abstractions. Modules that need HTTP add
 * deki-http to their requires.
 */
class DEKI_HTTP_API DekiHttp
{
public:
    static void             SetCurrent(IDekiHttpClient* client);
    static IDekiHttpClient* GetCurrent();

    // --- Convenience pass-throughs. Return empty/transport-error if no client ---

    static std::string FetchUrl(const std::string& url);

    static IDekiHttpClient::Response Get(const std::string& url,
                                         const IDekiHttpClient::HeaderList& headers = {},
                                         uint32_t timeoutMs = 15000);

    static IDekiHttpClient::Response PostJson(const std::string& url,
                                              const std::string& body,
                                              const IDekiHttpClient::HeaderList& headers = {},
                                              uint32_t timeoutMs = 15000);
};
