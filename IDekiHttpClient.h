#pragma once

#include <cstdint>   // uint32_t in the Get/PostJson signatures below
#include <string>
#include <vector>
#include <utility>

/**
 * @brief Abstract HTTP client interface.
 *
 * Concrete implementations live in platform integration packages and register
 * themselves with DekiHttp at boot. Consumers fetch the active client via
 * DekiHttp and call into the methods below; they never include any concrete
 * client header.
 *
 * Failure modes are uniform across implementations:
 *   - Network / DNS / TLS errors → Response.status == -1, body empty.
 *   - Non-2xx → Response.status carries the code, body may be empty or
 *     contain the server's error payload.
 *   Implementations log details on failure.
 */
class IDekiHttpClient
{
public:
    virtual ~IDekiHttpClient() = default;

    struct Response {
        int          status = -1;   // HTTP status, or -1 on transport error
        std::string  body;          // response body (may be empty on failure)
    };

    using HeaderList = std::vector<std::pair<std::string, std::string>>;

    /**
     * @brief Legacy synchronous GET. Returns the response body as a string.
     *        Returns an empty string on any failure (transport, non-2xx, etc.).
     *        Kept for backward compatibility with existing call sites.
     */
    virtual std::string FetchUrl(const std::string& url) = 0;

    /**
     * @brief Synchronous GET with custom request headers.
     *        Default implementation falls back to FetchUrl() and returns the
     *        body without status info; overriding gives status + body + header
     *        support. Implementations on real HTTP stacks override this.
     */
    virtual Response Get(const std::string& url,
                         const HeaderList&  headers   = {},
                         uint32_t           timeoutMs = 15000)
    {
        (void)headers; (void)timeoutMs;
        Response r;
        r.body   = FetchUrl(url);
        r.status = r.body.empty() ? -1 : 200;
        return r;
    }

    /**
     * @brief Synchronous POST with JSON body. Implementations set
     *        Content-Type: application/json automatically. Default returns
     *        a transport error so stubs / legacy clients that don't override
     *        fail loudly rather than silently succeeding.
     */
    virtual Response PostJson(const std::string& url,
                              const std::string& body,
                              const HeaderList&  headers   = {},
                              uint32_t           timeoutMs = 15000)
    {
        (void)url; (void)body; (void)headers; (void)timeoutMs;
        return {};
    }
};
