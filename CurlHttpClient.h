#pragma once

#include "IDekiHttpClient.h"

namespace DekiHttp
{

/// IDekiHttpClient that runs the curl program, for POSIX desktops (Linux,
/// macOS). Implements the whole interface: status codes, request headers and
/// JSON POST.
///
/// A subprocess rather than libcurl: this package is compiled into every
/// project that depends on it, and linking libcurl would make a missing
/// libcurl-dev fail the whole project's build. Run as a program, a missing
/// curl is only a logged transport error. The editor's EditorHttpUtils does
/// the same.
///
/// curl runs through fork/execvp with an argv array, never a shell, so URLs,
/// headers and bodies need no quoting and cannot inject commands.
class CurlHttpClient : public IDekiHttpClient
{
public:
    CurlHttpClient() = default;
    ~CurlHttpClient() override = default;

    std::string FetchUrl(const std::string& url) override;

    Response Get(const std::string& url, const HeaderList& headers = {}, uint32_t timeoutMs = 15000) override;

    Response PostJson(const std::string& url, const std::string& body, const HeaderList& headers = {},
                      uint32_t timeoutMs = 15000) override;
};

}  // namespace DekiHttp
