#include "DekiHttp.h"

namespace {
    IDekiHttpClient* s_Current = nullptr;
}

void DekiHttp::SetCurrent(IDekiHttpClient* client)
{
    s_Current = client;
}

IDekiHttpClient* DekiHttp::GetCurrent()
{
    return s_Current;
}

std::string DekiHttp::FetchUrl(const std::string& url)
{
    if (!s_Current) return "";
    return s_Current->FetchUrl(url);
}

IDekiHttpClient::Response DekiHttp::Get(const std::string& url,
                                         const IDekiHttpClient::HeaderList& headers,
                                         uint32_t timeoutMs)
{
    if (!s_Current) return {};
    return s_Current->Get(url, headers, timeoutMs);
}

IDekiHttpClient::Response DekiHttp::PostJson(const std::string& url,
                                              const std::string& body,
                                              const IDekiHttpClient::HeaderList& headers,
                                              uint32_t timeoutMs)
{
    if (!s_Current) return {};
    return s_Current->PostJson(url, body, headers, timeoutMs);
}
