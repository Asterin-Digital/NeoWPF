#include <gtest/gtest.h>

#include <NeoWPF/Diagnostics/Logger.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>

namespace {

struct OutputCapture {
    std::mutex mutex;
    std::condition_variable condition;
    std::string message;
};

OutputCapture& GetOutputCapture()
{
    static OutputCapture capture;
    return capture;
}

void CaptureOutput(const char* message) noexcept
{
    auto& capture = GetOutputCapture();
    {
        std::lock_guard<std::mutex> lock(capture.mutex);
        capture.message = message;
    }
    capture.condition.notify_one();
}

}

TEST(DiagnosticsTests, OutputHandlerReceivesFormattedMessages)
{
    constexpr char expectedMessage[] = "captured logger message 42";
    auto& capture = GetOutputCapture();
    {
        std::lock_guard<std::mutex> lock(capture.mutex);
        capture.message.clear();
    }

    const auto previousLevel = NeoWPF::Diagnostics::GetGlobalLevel();
    NeoWPF::Diagnostics::SetOutputHandler(CaptureOutput);
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Info);
    NeoWPF::Diagnostics::LogInfo("captured logger message %d", 42);

    std::string capturedMessage;
    bool received = false;
    {
        std::unique_lock<std::mutex> lock(capture.mutex);
        received = capture.condition.wait_for(lock, std::chrono::seconds(2), [&capture, &expectedMessage] {
            return capture.message.find(expectedMessage) != std::string::npos;
        });
        capturedMessage = capture.message;
    }

    NeoWPF::Diagnostics::SetOutputHandler(nullptr);
    NeoWPF::Diagnostics::SetGlobalLevel(previousLevel);

    EXPECT_TRUE(received);
    EXPECT_NE(capturedMessage.find(expectedMessage), std::string::npos);
}