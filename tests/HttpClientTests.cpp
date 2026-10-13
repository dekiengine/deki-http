// The desktop HTTP client against a real server on localhost: the platform's
// own client (WinHttpClient on Windows, CurlHttpClient elsewhere), so these are
// the requests a game makes, minus the internet.
//
// Every client has to fail the same way (IDekiHttpClient): -1 on a transport
// error, the status as sent otherwise, and FetchUrl empty on anything but 2xx.
// The server is cpp-httplib, which the editor already fetches.

#include <gtest/gtest.h>

#include <httplib.h>

#include "DekiHttp.h"
#ifdef _WIN32
#include "WinHttpClient.h"
#else
#include "CurlHttpClient.h"
#endif

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

using namespace DekiHttp;

namespace
{

#ifdef _WIN32
using PlatformClient = WinHttpClient;
#else
using PlatformClient = CurlHttpClient;
#endif

class HttpClientTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ends in a newline and digits on purpose: the curl client finds the
        // status after the body, and a body that looks like one must not move it.
        m_Server.Get("/hello", [](const httplib::Request&, httplib::Response& res)
                     { res.set_content("hello\n200", "text/plain"); });
        m_Server.Get("/missing", [](const httplib::Request&, httplib::Response& res)
                     {
                         res.status = 404;
                         res.set_content("no such page", "text/plain");
                     });
        m_Server.Get("/empty", [](const httplib::Request&, httplib::Response& res) { res.status = 204; });
        m_Server.Get("/header", [](const httplib::Request& req, httplib::Response& res)
                     { res.set_content(req.get_header_value("X-Deki"), "text/plain"); });
        m_Server.Get("/query", [](const httplib::Request& req, httplib::Response& res)
                     { res.set_content(req.get_param_value("q"), "text/plain"); });
        m_Server.Get("/moved", [](const httplib::Request&, httplib::Response& res) { res.set_redirect("/hello"); });
        // Answers only once the test is over (or after 30 s), so the client's
        // timeout is what ends the request, and teardown does not wait.
        m_Server.Get("/slow", [this](const httplib::Request&, httplib::Response& res)
                     {
                         std::unique_lock<std::mutex> lock(m_SlowMutex);
                         m_SlowCv.wait_for(lock, std::chrono::seconds(30), [this] { return m_SlowReleased; });
                         res.set_content("late", "text/plain");
                     });
        m_Server.Post("/echo", [](const httplib::Request& req, httplib::Response& res)
                      {
                          res.status = 201;
                          res.set_content(req.get_header_value("Content-Type") + "|" + req.body, "text/plain");
                      });

        m_Port = m_Server.bind_to_any_port("127.0.0.1");
        ASSERT_GT(m_Port, 0);
        m_Thread = std::thread([this] { m_Server.listen_after_bind(); });
        m_Server.wait_until_ready();
    }

    void TearDown() override
    {
        {
            std::lock_guard<std::mutex> lock(m_SlowMutex);
            m_SlowReleased = true;
        }
        m_SlowCv.notify_all();
        m_Server.stop();
        if (m_Thread.joinable())
        {
            m_Thread.join();
        }
    }

    std::string Url(const std::string& path) const { return "http://127.0.0.1:" + std::to_string(m_Port) + path; }

    PlatformClient client;

private:
    httplib::Server m_Server;
    std::thread m_Thread;
    int m_Port = 0;
    std::mutex m_SlowMutex;
    std::condition_variable m_SlowCv;
    bool m_SlowReleased = false;
};

}  // namespace

TEST_F(HttpClientTest, GetReturnsStatusAndBody)
{
    const auto r = client.Get(Url("/hello"));
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, "hello\n200");
}

TEST_F(HttpClientTest, ANon2xxStatusIsReportedWithItsBody)
{
    const auto r = client.Get(Url("/missing"));
    EXPECT_EQ(r.status, 404);
    EXPECT_EQ(r.body, "no such page");
}

TEST_F(HttpClientTest, AnEmptySuccessIsStillASuccess)
{
    const auto r = client.Get(Url("/empty"));
    EXPECT_EQ(r.status, 204);
    EXPECT_EQ(r.body, "");
}

TEST_F(HttpClientTest, FetchUrlIsEmptyOnAnythingButSuccess)
{
    EXPECT_EQ(client.FetchUrl(Url("/hello")), "hello\n200");
    EXPECT_EQ(client.FetchUrl(Url("/missing")), "");
}

TEST_F(HttpClientTest, RequestHeadersAreSent)
{
    const auto r = client.Get(Url("/header"), { { "X-Deki", "engine" } });
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, "engine");
}

TEST_F(HttpClientTest, TheQueryStringIsSent)
{
    const auto r = client.Get(Url("/query?q=value"));
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, "value");
}

TEST_F(HttpClientTest, RedirectsAreFollowed)
{
    const auto r = client.Get(Url("/moved"));
    EXPECT_EQ(r.status, 200);
    EXPECT_EQ(r.body, "hello\n200");
}

TEST_F(HttpClientTest, PostJsonSendsTheBodyAsJson)
{
    const std::string json = R"({"score":42,"name":"a \"quoted\" name"})";
    const auto r = client.PostJson(Url("/echo"), json);
    EXPECT_EQ(r.status, 201);
    EXPECT_EQ(r.body, "application/json|" + json);
}

TEST_F(HttpClientTest, NothingListeningIsATransportError)
{
    // Port 1 on localhost: reserved, and nothing listens there.
    const auto r = client.Get("http://127.0.0.1:1/hello", {}, 3000);
    EXPECT_EQ(r.status, -1);
    EXPECT_EQ(client.FetchUrl("http://127.0.0.1:1/hello"), "");
}

TEST_F(HttpClientTest, ATimeoutIsATransportError)
{
    // Not timed to the millisecond: WinHTTP was measured ending a 1 s request
    // after about 4 s. What matters is that the call returns as a transport
    // error instead of waiting on the server.
    const auto r = client.Get(Url("/slow"), {}, 1000);
    EXPECT_EQ(r.status, -1);
}

// --- The DekiHttp calls, which go to whichever client is current ---

namespace
{

class RecordingClient : public IDekiHttpClient
{
public:
    std::string lastUrl;
    std::string lastBody;
    HeaderList lastHeaders;
    uint32_t lastTimeout = 0;

    std::string FetchUrl(const std::string& url) override
    {
        lastUrl = url;
        return "fetched";
    }
    Response Get(const std::string& url, const HeaderList& headers, uint32_t timeoutMs) override
    {
        lastUrl = url;
        lastHeaders = headers;
        lastTimeout = timeoutMs;
        return { 200, "got" };
    }
    Response PostJson(const std::string& url, const std::string& body, const HeaderList& headers,
                      uint32_t timeoutMs) override
    {
        lastUrl = url;
        lastBody = body;
        lastHeaders = headers;
        lastTimeout = timeoutMs;
        return { 201, "posted" };
    }
};

class MinimalClient : public IDekiHttpClient
{
public:
    std::string body;
    std::string FetchUrl(const std::string&) override { return body; }
};

}  // namespace

TEST(DekiHttp, WithNoClientEveryCallFails)
{
    SetCurrent(nullptr);
    EXPECT_EQ(GetCurrent(), nullptr);
    EXPECT_EQ(FetchUrl("http://example.com"), "");
    EXPECT_EQ(Get("http://example.com").status, -1);
    EXPECT_EQ(PostJson("http://example.com", "{}").status, -1);
}

TEST(DekiHttp, CallsGoToTheCurrentClient)
{
    RecordingClient rec;
    SetCurrent(&rec);
    EXPECT_EQ(FetchUrl("u1"), "fetched");
    EXPECT_EQ(rec.lastUrl, "u1");

    const auto g = Get("u2", { { "A", "b" } }, 1234);
    EXPECT_EQ(g.status, 200);
    EXPECT_EQ(rec.lastUrl, "u2");
    ASSERT_EQ(rec.lastHeaders.size(), 1u);
    EXPECT_EQ(rec.lastHeaders[0].first, "A");
    EXPECT_EQ(rec.lastTimeout, 1234u);

    const auto p = PostJson("u3", "{}");
    EXPECT_EQ(p.status, 201);
    EXPECT_EQ(rec.lastBody, "{}");
    EXPECT_EQ(rec.lastTimeout, 15000u);  // the default
    SetCurrent(nullptr);
}

TEST(DekiHttp, TheInterfaceDefaultsFailVisibly)
{
    // A client that only implements FetchUrl: Get reports what it can, and
    // PostJson is a transport error rather than a silent success.
    MinimalClient minimal;
    minimal.body = "data";
    EXPECT_EQ(minimal.Get("u").status, 200);
    EXPECT_EQ(minimal.Get("u").body, "data");
    minimal.body.clear();
    EXPECT_EQ(minimal.Get("u").status, -1);
    EXPECT_EQ(minimal.PostJson("u", "{}").status, -1);
}
