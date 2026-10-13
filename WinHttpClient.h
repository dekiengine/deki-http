#pragma once

#include "IDekiHttpClient.h"

namespace DekiHttp
{

/// IDekiHttpClient on the Windows WinHTTP API, for the editor and desktop
/// builds; boards have their own HTTP integration. Implements the whole
/// interface, as CurlHttpClient does on POSIX: status codes, request headers
/// and JSON POST.
class WinHttpClient : public IDekiHttpClient
{
public:
    WinHttpClient() = default;
    ~WinHttpClient() override = default;

    std::string FetchUrl(const std::string& url) override;

    Response Get(const std::string& url, const HeaderList& headers = {}, uint32_t timeoutMs = 15000) override;

    Response PostJson(const std::string& url, const std::string& body, const HeaderList& headers = {},
                      uint32_t timeoutMs = 15000) override;
};

}  // namespace DekiHttp
