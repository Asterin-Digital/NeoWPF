#include <NeoWPF/Diagnostics/Logger.h>
#include <windows.h>
#include <winhttp.h>
#include <vector>
#include <string>

#pragma comment(lib, "Winhttp.lib")

namespace NeoWPF::Platform {

bool FetchHttpResource(const std::wstring& url, std::vector<uint8_t>& outData) {
    HINTERNET hSession = WinHttpOpen(L"NeoWPF/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        NeoWPF::Diagnostics::LogError("FetchHttpResource: WinHttpOpen failed");
        return false;
    }

    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength = (DWORD)-1;
    urlComp.dwHostNameLength = (DWORD)-1;
    urlComp.dwUrlPathLength = (DWORD)-1;

    if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComp)) {
        NeoWPF::Diagnostics::LogError("FetchHttpResource: WinHttpCrackUrl failed for URL: %ls, error: %lu", url.c_str(), GetLastError());
        WinHttpCloseHandle(hSession);
        return false;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Http, NeoWPF::Diagnostics::Level::Info, "Parsed URL - scheme: %u, host length: %u, port: %u, path length: %u",
        urlComp.nScheme, urlComp.dwHostNameLength, urlComp.nPort, urlComp.dwUrlPathLength);


    std::wstring hostname(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

    HINTERNET hConnect = WinHttpConnect(hSession, hostname.c_str(), urlComp.nPort, 0);
    if (!hConnect) {
        NeoWPF::Diagnostics::LogError("FetchHttpResource: WinHttpConnect failed for host '%ls' (port %u), error: %lu", hostname.c_str(), urlComp.nPort, GetLastError());
        WinHttpCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath.c_str(),
                                            nullptr, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            urlComp.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        NeoWPF::Diagnostics::LogError("FetchHttpResource: WinHttpOpenRequest failed");
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0);
    if (bResults) bResults = WinHttpReceiveResponse(hRequest, nullptr);

    if (!bResults) {
        NeoWPF::Diagnostics::LogError("FetchHttpResource: WinHttpSendRequest or WinHttpReceiveResponse failed, error: %lu", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    if (bResults) {
        DWORD dwSize = 0;
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;
            std::vector<uint8_t> buffer(dwSize);
            DWORD dwRead = 0;
            if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwRead)) break;
            outData.insert(outData.end(), buffer.begin(), buffer.begin() + dwRead);
        } while (true);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (outData.empty()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Http, NeoWPF::Diagnostics::Level::Info, "No data received from URL: %ls", url.c_str());
    } else {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Http, NeoWPF::Diagnostics::Level::Info, "Successfully fetched %u bytes", static_cast<unsigned int>(outData.size()));
    }
    return bResults && !outData.empty();
}

}
