#include <NeoWPF/Platform/CurlHttp.h>

#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <curl/curl.h>

#include <limits>
#include <mutex>

namespace NeoWPF::Platform {
namespace {

constexpr std::size_t kMaximumResponseSize = 64u * 1024u * 1024u;
constexpr long kConnectTimeoutSeconds = 10;
constexpr long kTransferTimeoutSeconds = 30;

struct CurlBuffer {
    std::vector<std::uint8_t>* data = nullptr;
};

std::size_t WriteResponse(char* contents, std::size_t size, std::size_t count, void* userData)
{
    if (count != 0 && size > std::numeric_limits<std::size_t>::max() / count) {
        return 0;
    }
    const std::size_t bytes = size * count;
    auto* buffer = static_cast<CurlBuffer*>(userData);
    if (!buffer || !buffer->data || bytes > kMaximumResponseSize - buffer->data->size()) {
        return 0;
    }

    buffer->data->insert(buffer->data->end(), contents, contents + bytes);
    return bytes;
}

bool InitializeCurl()
{
    static std::once_flag once;
    static bool initialized = false;
    std::call_once(once, [] { initialized = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK; });
    return initialized;
}

}

bool CurlHttp::Fetch(const std::wstring& url, std::vector<std::uint8_t>& outData) const
{
    outData.clear();
    if (url.empty() || !InitializeCurl()) {
        return false;
    }

    CURL* handle = curl_easy_init();
    if (!handle) {
        NeoWPF::Diagnostics::LogError("CurlHttp: curl_easy_init failed");
        return false;
    }

    const std::string urlUtf8 = NeoWPF::Platform::WideToUtf8(url);
    CurlBuffer buffer{&outData};
    curl_easy_setopt(handle, CURLOPT_URL, urlUtf8.c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, kConnectTimeoutSeconds);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, kTransferTimeoutSeconds);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteResponse);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "NeoWPF/1.0");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 2L);

    const CURLcode result = curl_easy_perform(handle);
    long responseCode = 0;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_cleanup(handle);

    if (result != CURLE_OK || responseCode < 200 || responseCode >= 300 || outData.empty()) {
        outData.clear();
        NeoWPF::Diagnostics::LogError(
            "CurlHttp: request failed for URL '%s' (curl=%d, HTTP=%ld)",
            urlUtf8.c_str(), static_cast<int>(result), responseCode);
        return false;
    }

    return true;
}

}