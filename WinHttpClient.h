#pragma once

#include "IDekiHttpClient.h"

namespace DekiHttp
{

/// IDekiHttpClient on the Windows WinHTTP API, with blocking GET. Used by the
/// editor and desktop builds; boards have their own HTTP integration.
///
/// Only FetchUrl is implemented. Get and PostJson use the IDekiHttpClient
/// defaults: Get goes through FetchUrl, and PostJson returns a transport error.
class WinHttpClient : public IDekiHttpClient
{
public:
    WinHttpClient() = default;
    ~WinHttpClient() override = default;

    std::string FetchUrl(const std::string& url) override;
};

}  // namespace DekiHttp
