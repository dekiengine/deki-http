#include "WinHttpClient.h"
#include <deki/LogSystem.h>

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#endif

namespace DekiHttp
{

#ifdef _WIN32

namespace
{

// UTF-8 to UTF-16. URLs and header values are ASCII in practice, but a widened
// byte-per-char copy would mangle anything that is not.
std::wstring Widen(const std::string& s)
{
    if (s.empty())
    {
        return {};
    }
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

/// One request, GET or POST, with the status, as every call needs. `body`
/// null means no request body.
IDekiHttpClient::Response Request(const wchar_t* method, const std::string& url,
                                  const IDekiHttpClient::HeaderList& headers, const std::string* body,
                                  uint32_t timeoutMs)
{
    IDekiHttpClient::Response response;
    const std::wstring wUrl = Widen(url);

    URL_COMPONENTS urlComp{};
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostName[256] = { 0 };
    wchar_t urlPath[2048] = { 0 };
    wchar_t extraInfo[2048] = { 0 };
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);
    urlComp.lpszExtraInfo = extraInfo;  // the query string, which the path leaves out
    urlComp.dwExtraInfoLength = sizeof(extraInfo) / sizeof(wchar_t);

    if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp))
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpCrackUrl failed for %s", url.c_str());
        return response;
    }
    const std::wstring target = std::wstring(urlPath) + extraInfo;

    HINTERNET hSession = WinHttpOpen(L"DekiEngine/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpOpen failed");
        return response;
    }

    const int timeout = static_cast<int>(timeoutMs);
    WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);

    HINTERNET hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
    HINTERNET hRequest = nullptr;
    if (hConnect)
    {
        const DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        hRequest = WinHttpOpenRequest(hConnect, method, target.c_str(), nullptr, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    }
    if (!hRequest)
    {
        DEKI_LOG_WARNING("[deki-http] could not open a request for %s", url.c_str());
        if (hConnect)
        {
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
        return response;
    }

    std::wstring headerBlock;
    for (const auto& [name, value] : headers)
    {
        headerBlock += Widen(name) + L": " + Widen(value) + L"\r\n";
    }
    if (body)
    {
        headerBlock += L"Content-Type: application/json\r\n";
    }

    BOOL ok = WinHttpSendRequest(hRequest, headerBlock.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headerBlock.c_str(),
                                 headerBlock.empty() ? 0 : static_cast<DWORD>(-1),
                                 body ? const_cast<char*>(body->data()) : WINHTTP_NO_REQUEST_DATA,
                                 body ? static_cast<DWORD>(body->size()) : 0,
                                 body ? static_cast<DWORD>(body->size()) : 0, 0);
    if (ok)
    {
        ok = WinHttpReceiveResponse(hRequest, nullptr);
    }

    if (ok)
    {
        DWORD status = 0;
        DWORD size = sizeof(status);
        if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX))
        {
            response.status = static_cast<int>(status);
        }

        char buffer[4096];
        DWORD bytesRead = 0;
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        {
            response.body.append(buffer, bytesRead);
            if (response.body.size() > 16 * 1024 * 1024)
            {
                break;  // 16 MB cap
            }
        }
    }
    else
    {
        DEKI_LOG_WARNING("[deki-http] Request failed for %s (error %lu)", url.c_str(), GetLastError());
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

}  // namespace

std::string WinHttpClient::FetchUrl(const std::string& url)
{
    const Response r = Get(url);
    // FetchUrl returns an empty string on any failure, including non-2xx.
    if (r.status < 200 || r.status > 299)
    {
        return "";
    }
    return r.body;
}

IDekiHttpClient::Response WinHttpClient::Get(const std::string& url, const HeaderList& headers, uint32_t timeoutMs)
{
    return Request(L"GET", url, headers, nullptr, timeoutMs);
}

IDekiHttpClient::Response WinHttpClient::PostJson(const std::string& url, const std::string& body,
                                                  const HeaderList& headers, uint32_t timeoutMs)
{
    return Request(L"POST", url, headers, &body, timeoutMs);
}

#else

std::string WinHttpClient::FetchUrl(const std::string& /*url*/)
{
    DEKI_LOG_ERROR("[deki-http] WinHttpClient called on non-Windows platform");
    return "";
}

IDekiHttpClient::Response WinHttpClient::Get(const std::string&, const HeaderList&, uint32_t)
{
    DEKI_LOG_ERROR("[deki-http] WinHttpClient called on non-Windows platform");
    return {};
}

IDekiHttpClient::Response WinHttpClient::PostJson(const std::string&, const std::string&, const HeaderList&, uint32_t)
{
    DEKI_LOG_ERROR("[deki-http] WinHttpClient called on non-Windows platform");
    return {};
}

#endif

}  // namespace DekiHttp
