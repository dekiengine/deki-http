#pragma once

#include "IDekiHttpClient.h"

/**
 * curl-backed IDekiHttpClient implementation for POSIX desktops (Linux, macOS).
 *
 * Why a subprocess and not libcurl: this module is compiled as part of every
 * project that depends on it, so a hard link-time dependency on libcurl would
 * turn "libcurl-dev isn't installed" into a build failure for the whole
 * project. Shelling out keeps the dependency at runtime, where a missing curl
 * degrades to a logged transport error instead. The editor's own
 * EditorHttpUtils made the same trade.
 *
 * curl is invoked via fork/execvp with an argv array — never through a shell —
 * so URLs, headers and bodies need no quoting and cannot inject commands.
 *
 * Implements the full interface: status codes, request headers, and JSON POST.
 */
class CurlHttpClient : public IDekiHttpClient
{
public:
    CurlHttpClient()  = default;
    ~CurlHttpClient() override = default;

    std::string FetchUrl(const std::string& url) override;

    Response Get(const std::string& url,
                 const HeaderList&  headers   = {},
                 uint32_t           timeoutMs = 15000) override;

    Response PostJson(const std::string& url,
                      const std::string& body,
                      const HeaderList&  headers   = {},
                      uint32_t           timeoutMs = 15000) override;
};
