#include "WinHttpClient.h"
#include "DekiLogSystem.h"

#ifdef _WIN32

#include <windows.h>
#include <winhttp.h>

std::string WinHttpClient::FetchUrl(const std::string& url)
{
    std::wstring wUrl(url.begin(), url.end());

    URL_COMPONENTS urlComp{};
    urlComp.dwStructSize = sizeof(urlComp);
    wchar_t hostName[256] = {0};
    wchar_t urlPath[2048] = {0};
    urlComp.lpszHostName    = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
    urlComp.lpszUrlPath     = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

    if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp))
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpCrackUrl failed for %s", url.c_str());
        return "";
    }

    HINTERNET hSession = WinHttpOpen(L"DekiEngine/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpOpen failed");
        return "";
    }

    WinHttpSetTimeouts(hSession, 5000, 5000, 5000, 5000);

    HINTERNET hConnect = WinHttpConnect(hSession, hostName, urlComp.nPort, 0);
    if (!hConnect)
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpConnect failed");
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest)
    {
        DEKI_LOG_WARNING("[deki-http] WinHttpOpenRequest failed");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    WinHttpAddRequestHeaders(hRequest, L"User-Agent: DekiEngine/1.0", -1, WINHTTP_ADDREQ_FLAG_ADD);

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (ok) ok = WinHttpReceiveResponse(hRequest, NULL);

    std::string result;
    if (ok)
    {
        char buffer[4096];
        DWORD bytesRead = 0;
        while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
        {
            result.append(buffer, bytesRead);
            if (result.size() > 16 * 1024 * 1024) break; // 16 MB cap
        }
    }
    else
    {
        DEKI_LOG_WARNING("[deki-http] Request failed for %s", url.c_str());
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

#else

std::string WinHttpClient::FetchUrl(const std::string& /*url*/)
{
    DEKI_LOG_ERROR("[deki-http] WinHttpClient called on non-Windows platform");
    return "";
}

#endif
