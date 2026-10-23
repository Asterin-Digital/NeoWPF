#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>

namespace {

class WindowSessionGuard {
public:
    explicit WindowSessionGuard(void* nativeWindowHandle)
        : m_nativeWindowHandle(nativeWindowHandle)
    {
        NeoWPF::Core::WindowSession session;
        session.nativeWindowHandle = nativeWindowHandle;
        session.windowHost = std::make_shared<NeoWPF::UI::Window>();

        auto& appState = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        appState.windowSessions.push_back(std::move(session));
    }

    ~WindowSessionGuard()
    {
        NeoWPF::Core::CloseApplicationWindow(m_nativeWindowHandle);
    }

private:
    void* m_nativeWindowHandle;
};

bool HasWindowSession(void* nativeWindowHandle)
{
    auto& appState = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
    return std::any_of(appState.windowSessions.begin(), appState.windowSessions.end(),
        [nativeWindowHandle](const NeoWPF::Core::WindowSession& session) {
            return session.nativeWindowHandle == nativeWindowHandle;
        });
}

TEST(ApplicationWindowSessionTests, CloseRejectsNullHandle)
{
    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(nullptr));
}

TEST(ApplicationWindowSessionTests, CloseRejectsUnknownHandleAndPreservesRegisteredSession)
{
    int registeredHandle = 0;
    int unknownHandle = 0;
    WindowSessionGuard session(&registeredHandle);

    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(&unknownHandle));
    EXPECT_TRUE(HasWindowSession(&registeredHandle));
}

TEST(ApplicationWindowSessionTests, CloseRemovesRegisteredSessionOnlyOnce)
{
    int nativeHandle = 0;
    WindowSessionGuard session(&nativeHandle);

    EXPECT_TRUE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));
    EXPECT_FALSE(HasWindowSession(&nativeHandle));
    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));
}

}
