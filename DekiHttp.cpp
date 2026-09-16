#include "DekiHttp.h"

namespace DekiHttp
{

namespace {
    IDekiHttpClient* s_Current = nullptr;
}

void SetCurrent(IDekiHttpClient* client)
{
    s_Current = client;
}

IDekiHttpClient* GetCurrent()
{
    return s_Current;
}

std::string FetchUrl(const std::string& url)
{
    if (!s_Current) return "";
    return s_Current->FetchUrl(url);
}

IDekiHttpClient::Response Get(const std::string& url,
                                         const IDekiHttpClient::HeaderList& headers,
                                         uint32_t timeoutMs)
{
    if (!s_Current) return {};
    return s_Current->Get(url, headers, timeoutMs);
}

IDekiHttpClient::Response PostJson(const std::string& url,
                                              const std::string& body,
                                              const IDekiHttpClient::HeaderList& headers,
                                              uint32_t timeoutMs)
{
    if (!s_Current) return {};
    return s_Current->PostJson(url, body, headers, timeoutMs);
}

}  // namespace DekiHttp
