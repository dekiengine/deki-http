#pragma once

#include <cstdint>  // uint32_t in the timeout parameters below
#include "IDekiHttpClient.h"
#include "DekiHttpPackage.h"

namespace DekiHttp
{

/// The active HTTP client and calls that go to it.
///
/// A platform integration package registers its IDekiHttpClient with
/// SetCurrent() during setup. Users call DekiHttp::FetchUrl / Get / PostJson;
/// the client lives in deki-http.dll and every package using it imports it.
///
/// It is a package, not part of deki-engine-core, so the engine core has no
/// service implementations. Packages that need HTTP list deki-http in their
/// requires.
DEKI_HTTP_API void SetCurrent(IDekiHttpClient* client);
DEKI_HTTP_API IDekiHttpClient* GetCurrent();

// --- Calls passed to the current client; empty or a transport error without one ---

DEKI_HTTP_API std::string FetchUrl(const std::string& url);

DEKI_HTTP_API IDekiHttpClient::Response Get(const std::string& url, const IDekiHttpClient::HeaderList& headers = {},
                                            uint32_t timeoutMs = 15000);

DEKI_HTTP_API IDekiHttpClient::Response PostJson(const std::string& url, const std::string& body,
                                                 const IDekiHttpClient::HeaderList& headers = {},
                                                 uint32_t timeoutMs = 15000);

}  // namespace DekiHttp
