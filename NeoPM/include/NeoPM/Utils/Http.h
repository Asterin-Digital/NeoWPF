/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace NeoPM::Net
{
    inline constexpr const char* NEOWPF_SDK_RELEASES_URL =
        "https://raw.githubusercontent.com/NeoWPF/NeoWPF/main/sdk-releases.json";

    using DownloadProgressCallback = std::function<bool(uint64_t bytesReceived, uint64_t totalBytes)>;

    struct DownloadOptions
    {
        std::wstring userAgent = L"NeoPM/1.0";
        uint32_t resolveTimeoutMs = 10000;
        uint32_t connectTimeoutMs = 10000;
        uint32_t sendTimeoutMs = 10000;
        uint32_t receiveTimeoutMs = 30000;
        int maxRetries = 3;
        uint32_t retryBackoffMs = 1000;
        std::vector<std::wstring> extraHeaders;
    };

    enum class DownloadResult
    {
        Success,
        InvalidUrl,
        ConnectionFailed,
        TlsError,
        HttpError,
        Timeout,
        IoError,
        Cancelled,
        Unknown
    };

    struct DownloadOutcome
    {
        DownloadResult result = DownloadResult::Unknown;
        uint32_t httpStatusCode = 0;
        uint32_t win32Error = 0;
        std::wstring message;
        uint64_t bytesWritten = 0;
        int attemptsMade = 0;

        bool Ok() const { return result == DownloadResult::Success; }
    };

    class CurlClient
    {
    public:
        explicit CurlClient(DownloadOptions options = {});
        ~CurlClient();

        CurlClient(const CurlClient&) = delete;
        CurlClient& operator=(const CurlClient&) = delete;
        CurlClient(CurlClient&&) = delete;
        CurlClient& operator=(CurlClient&&) = delete;

        DownloadOutcome DownloadFile(const std::wstring& url,
                                     const std::filesystem::path& destPath,
                                     const DownloadProgressCallback& onProgress = nullptr);

        DownloadOutcome DownloadToBuffer(const std::wstring& url,
                                         std::string& outBuffer,
                                         const DownloadProgressCallback& onProgress = nullptr);

        static bool HasInternetConnectivity();

    private:
        DownloadOutcome ExecuteRequest(
            const std::wstring& url,
            const DownloadProgressCallback& onProgress,
            const std::function<bool(const char* data, size_t len)>& onChunk,
            uint64_t& bytesReceivedOut);

        static bool IsTransient(const DownloadOutcome& outcome);

        DownloadOptions m_options;
        void* m_handle = nullptr;
    };
}
