#pragma once

#include <cstdint>  // uint32_t in the timeout parameters below
#include "IDekiHttpClient.h"
#include "DekiHttpPackage.h"

namespace DekiHttp
{

/**
 * @brief Active-driver registry and facade for HTTP.
 *
 * A platform integration package registers its IDekiHttpClient implementation
 * via SetCurrent() during its setup phase. Consumers call DekiHttp::FetchUrl
 * / Get / PostJson directly — the singleton is dllexported from deki-http.dll
 * and imported by every consumer package.
 *
 * Lives in deki-http.dll (moved out of deki-engine-core) so that engine-core
 * stays free of concrete service abstractions. Packages that need HTTP add
 * deki-http to their requires.
 */
DEKI_HTTP_API void             SetCurrent(IDekiHttpClient* client);
DEKI_HTTP_API IDekiHttpClient* GetCurrent();

// --- Convenience pass-throughs. Return empty/transport-error if no client ---

DEKI_HTTP_API std::string FetchUrl(const std::string& url);

DEKI_HTTP_API IDekiHttpClient::Response Get(const std::string& url,
                                     const IDekiHttpClient::HeaderList& headers = {},
                                     uint32_t timeoutMs = 15000);

DEKI_HTTP_API IDekiHttpClient::Response PostJson(const std::string& url,
                                          const std::string& body,
                                          const IDekiHttpClient::HeaderList& headers = {},
                                          uint32_t timeoutMs = 15000);

}  // namespace DekiHttp
