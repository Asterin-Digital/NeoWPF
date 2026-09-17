#include <NeoPM/Utils/Http.h>

#include <curl/curl.h>

#include <chrono>
#include <codecvt>
#include <fstream>
#include <locale>
#include <thread>

namespace NeoPM::Net
{
    namespace
    {
        std::string ToUtf8(const std::wstring& value)
        {
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(value);
        }

        std::wstring FromUtf8(const char* value)
        {
            return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(value);
        }

        struct CurlRuntime
        {
            CurlRuntime() { curl_global_init(CURL_GLOBAL_DEFAULT); }
            ~CurlRuntime() { curl_global_cleanup(); }
        };

        CurlRuntime& Runtime()
        {
            static CurlRuntime runtime;
            return runtime;
        }

        struct RequestContext
        {
            const DownloadProgressCallback* onProgress = nullptr;
            const std::function<bool(const char* data, size_t len)>* onChunk = nullptr;
            uint64_t bytesReceived = 0;
            bool callbackCancelled = false;
            bool writeFailed = false;
        };

        size_t WriteCallback(char* data, size_t size, size_t count, void* userData)
        {
            auto& context = *static_cast<RequestContext*>(userData);
            const auto bytes = size * count;
            if (!(*context.onChunk)(data, bytes))
            {
                context.writeFailed = true;
                return 0;
            }
            context.bytesReceived += bytes;
            return bytes;
        }

        int ProgressCallback(void* userData,
                             curl_off_t totalBytes,
                             curl_off_t bytesReceived,
                             curl_off_t,
                             curl_off_t)
        {
            auto& context = *static_cast<RequestContext*>(userData);
            if (context.onProgress && !(*context.onProgress)(
                    static_cast<uint64_t>(bytesReceived),
                    static_cast<uint64_t>(totalBytes)))
            {
                context.callbackCancelled = true;
                return 1;
            }
            return 0;
        }

        DownloadResult ClassifyError(CURLcode code)
        {
            switch (code)
            {
                case CURLE_URL_MALFORMAT:
                    return DownloadResult::InvalidUrl;
                case CURLE_OPERATION_TIMEDOUT:
                    return DownloadResult::Timeout;
                case CURLE_SSL_CONNECT_ERROR:
                case CURLE_PEER_FAILED_VERIFICATION:
                    return DownloadResult::TlsError;
                case CURLE_WRITE_ERROR:
                    return DownloadResult::IoError;
                case CURLE_ABORTED_BY_CALLBACK:
                    return DownloadResult::Cancelled;
                case CURLE_COULDNT_RESOLVE_HOST:
                case CURLE_COULDNT_CONNECT:
                case CURLE_RECV_ERROR:
                case CURLE_SEND_ERROR:
                case CURLE_GOT_NOTHING:
                case CURLE_PARTIAL_FILE:
                    return DownloadResult::ConnectionFailed;
                default:
                    return DownloadResult::Unknown;
            }
        }

        DownloadOutcome MakeCurlError(CURLcode code, const RequestContext& context)
        {
            DownloadOutcome outcome;
            outcome.result = context.writeFailed ? DownloadResult::IoError :
                context.callbackCancelled ? DownloadResult::Cancelled : ClassifyError(code);
            outcome.win32Error = static_cast<uint32_t>(code);
            outcome.message = FromUtf8(curl_easy_strerror(code));
            outcome.bytesWritten = context.bytesReceived;
            return outcome;
        }
    }

    CurlClient::CurlClient(DownloadOptions options)
        : m_options(std::move(options))
    {
        Runtime();
        m_handle = curl_easy_init();
    }

    CurlClient::~CurlClient()
    {
        if (m_handle)
            curl_easy_cleanup(static_cast<CURL*>(m_handle));
    }

    bool CurlClient::IsTransient(const DownloadOutcome& outcome)
    {
        return outcome.result == DownloadResult::Timeout ||
               outcome.result == DownloadResult::ConnectionFailed;
    }

    bool CurlClient::HasInternetConnectivity()
    {
        Runtime();
        auto* handle = curl_easy_init();
        if (!handle)
            return false;

        curl_easy_setopt(handle, CURLOPT_URL, "https://github.com/NeoWPF/NeoWPF");
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
        curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, 8000L);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "NeoPM/1.0");

        const auto result = curl_easy_perform(handle);
        long statusCode = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &statusCode);
        curl_easy_cleanup(handle);

        return result == CURLE_OK && statusCode >= 200 && statusCode < 400;
    }

    DownloadOutcome CurlClient::ExecuteRequest(
        const std::wstring& url,
        const DownloadProgressCallback& onProgress,
        const std::function<bool(const char* data, size_t len)>& onChunk,
        uint64_t& bytesReceivedOut)
    {
        bytesReceivedOut = 0;
        if (!m_handle)
        {
            DownloadOutcome outcome;
            outcome.result = DownloadResult::Unknown;
            outcome.message = L"curl_easy_init failed";
            return outcome;
        }

        auto* handle = static_cast<CURL*>(m_handle);
        auto urlUtf8 = ToUtf8(url);
        auto userAgentUtf8 = ToUtf8(m_options.userAgent);
        curl_slist* headers = nullptr;
        for (const auto& header : m_options.extraHeaders)
        {
            auto headerUtf8 = ToUtf8(header);
            headers = curl_slist_append(headers, headerUtf8.c_str());
        }

        RequestContext context{&onProgress, &onChunk};
        curl_easy_reset(handle);
        curl_easy_setopt(handle, CURLOPT_URL, urlUtf8.c_str());
        curl_easy_setopt(handle, CURLOPT_USERAGENT, userAgentUtf8.c_str());
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle, CURLOPT_FAILONERROR, 0L);
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(m_options.connectTimeoutMs));
        curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS,
                         static_cast<long>(m_options.resolveTimeoutMs +
                                           m_options.connectTimeoutMs +
                                           m_options.sendTimeoutMs +
                                           m_options.receiveTimeoutMs));
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &context);
        curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(handle, CURLOPT_XFERINFODATA, &context);
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);

        const auto code = curl_easy_perform(handle);
        curl_slist_free_all(headers);

        bytesReceivedOut = context.bytesReceived;
        if (code != CURLE_OK)
            return MakeCurlError(code, context);

        long statusCode = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &statusCode);
        if (statusCode < 200 || statusCode >= 300)
        {
            DownloadOutcome outcome;
            outcome.result = DownloadResult::HttpError;
            outcome.httpStatusCode = static_cast<uint32_t>(statusCode);
            outcome.message = L"HTTP " + std::to_wstring(statusCode) + L" from server";
            outcome.bytesWritten = context.bytesReceived;
            return outcome;
        }

        DownloadOutcome outcome;
        outcome.result = DownloadResult::Success;
        outcome.httpStatusCode = static_cast<uint32_t>(statusCode);
        outcome.bytesWritten = context.bytesReceived;
        return outcome;
    }

    DownloadOutcome CurlClient::DownloadFile(
        const std::wstring& url,
        const std::filesystem::path& destPath,
        const DownloadProgressCallback& onProgress)
    {
        std::filesystem::path tempPath = destPath;
        tempPath += L".part";

        std::error_code ec;
        std::filesystem::create_directories(destPath.parent_path(), ec);

        DownloadOutcome outcome;
        for (int attempt = 1; attempt <= m_options.maxRetries + 1; ++attempt)
        {
            outcome.attemptsMade = attempt;
            std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
            if (!out)
            {
                outcome.result = DownloadResult::IoError;
                outcome.message = L"Could not open temp file for writing: " + tempPath.wstring();
                return outcome;
            }

            auto onChunk = [&out](const char* data, size_t len) -> bool
            {
                out.write(data, static_cast<std::streamsize>(len));
                return static_cast<bool>(out);
            };

            uint64_t bytesReceived = 0;
            outcome = ExecuteRequest(url, onProgress, onChunk, bytesReceived);
            outcome.attemptsMade = attempt;
            out.close();

            if (outcome.Ok())
            {
                std::error_code renameEc;
                std::filesystem::rename(tempPath, destPath, renameEc);
                if (renameEc)
                {
                    std::filesystem::remove(destPath, renameEc);
                    std::filesystem::rename(tempPath, destPath, renameEc);
                }
                if (renameEc)
                {
                    outcome.result = DownloadResult::IoError;
                    outcome.message = L"Downloaded OK but failed to finalize file";
                }
                return outcome;
            }

            std::filesystem::remove(tempPath, ec);
            if (!IsTransient(outcome) || attempt > m_options.maxRetries)
                return outcome;

            std::this_thread::sleep_for(std::chrono::milliseconds(
                m_options.retryBackoffMs * (1u << (attempt - 1))));
        }
        return outcome;
    }

    DownloadOutcome CurlClient::DownloadToBuffer(
        const std::wstring& url,
        std::string& outBuffer,
        const DownloadProgressCallback& onProgress)
    {
        DownloadOutcome outcome;
        for (int attempt = 1; attempt <= m_options.maxRetries + 1; ++attempt)
        {
            outcome.attemptsMade = attempt;
            outBuffer.clear();

            auto onChunk = [&outBuffer](const char* data, size_t len) -> bool
            {
                outBuffer.append(data, len);
                return true;
            };

            uint64_t bytesReceived = 0;
            outcome = ExecuteRequest(url, onProgress, onChunk, bytesReceived);
            outcome.attemptsMade = attempt;
            if (outcome.Ok())
                return outcome;

            if (!IsTransient(outcome) || attempt > m_options.maxRetries)
                return outcome;

            std::this_thread::sleep_for(std::chrono::milliseconds(
                m_options.retryBackoffMs * (1u << (attempt - 1))));
        }
        return outcome;
    }
}
