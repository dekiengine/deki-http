#pragma once

#include <cstdint>  // uint32_t in the Get/PostJson signatures below
#include <string>
#include <vector>
#include <utility>

namespace DekiHttp
{

/// HTTP client interface.
///
/// Implementations live in platform integration packages and register with
/// DekiHttp at boot. Users get the active client through DekiHttp and never
/// include an implementation's header.
///
/// Every implementation fails the same way:
///   - Network, DNS or TLS error: Response.status is -1, body empty.
///   - Non-2xx: Response.status holds the code; the body may be empty or hold
///     the server's error payload.
/// Implementations log the details.
class IDekiHttpClient
{
public:
    virtual ~IDekiHttpClient() = default;

    struct Response
    {
        int status = -1;   // HTTP status, or -1 on transport error
        std::string body;  // response body (may be empty on failure)
    };

    using HeaderList = std::vector<std::pair<std::string, std::string>>;

    /// Blocking GET that returns the body, or an empty string on any failure
    /// (transport error, non-2xx). The older call; Get() also gives the status.
    virtual std::string FetchUrl(const std::string& url) = 0;

    /// Blocking GET with request headers. The default calls FetchUrl(),
    /// ignores the headers, and reports 200 or -1; real HTTP stacks override it
    /// to give the status, body and headers.
    virtual Response Get(const std::string& url, const HeaderList& headers = {}, uint32_t timeoutMs = 15000)
    {
        (void)headers;
        (void)timeoutMs;
        Response r;
        r.body = FetchUrl(url);
        r.status = r.body.empty() ? -1 : 200;
        return r;
    }

    /// Blocking POST of a JSON body; implementations set Content-Type:
    /// application/json. The default returns a transport error, so a client
    /// that does not override it fails visibly instead of seeming to succeed.
    virtual Response PostJson(const std::string& url, const std::string& body, const HeaderList& headers = {},
                              uint32_t timeoutMs = 15000)
    {
        (void)url;
        (void)body;
        (void)headers;
        (void)timeoutMs;
        return {};
    }
};

}  // namespace DekiHttp
