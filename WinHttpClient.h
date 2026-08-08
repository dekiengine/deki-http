#pragma once

#include "IDekiHttpClient.h"

/**
 * WinHTTP-backed IDekiHttpClient implementation. Synchronous GET via the
 * Windows WinHTTP API. Used by the editor / desktop builds; embedded boards
 * have their own platform-specific HTTP integration.
 *
 * Only FetchUrl is implemented today. Get(headers) / PostJson use the
 * IDekiHttpClient default fallback, which routes Get through FetchUrl and
 * returns a transport error for PostJson. Extending the WinHTTP backend with
 * full header / POST support can happen lazily when a desktop consumer needs it.
 */
class WinHttpClient : public IDekiHttpClient
{
public:
    WinHttpClient()  = default;
    ~WinHttpClient() override = default;

    std::string FetchUrl(const std::string& url) override;
};
