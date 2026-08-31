#include <gtest/gtest.h>

#ifdef _WIN32
#include <NeoWPF/Platform/Windows/WWindowChrome.h>
#include <windows.h>
#endif

namespace {

#ifdef _WIN32
TEST(WindowChromeTests, DefaultModeIsNativeAndCustomIsSupported)
{
    NeoWPF::Platform::Windows::WWindowChrome chrome;

    EXPECT_EQ(chrome.DefaultChromeMode(), NeoWPF::Contracts::ChromeMode::Native);
    EXPECT_TRUE(chrome.SupportsMode(NeoWPF::Contracts::ChromeMode::Native));
    EXPECT_TRUE(chrome.SupportsMode(NeoWPF::Contracts::ChromeMode::Custom));
    EXPECT_FALSE(chrome.SupportsMode(NeoWPF::Contracts::ChromeMode::None));
}

TEST(WindowChromeTests, StoresModeAndTitleBarHitTestCallback)
{
    NeoWPF::Platform::Windows::WWindowChrome chrome;

    HWND hwnd = CreateWindowExW(
        0,
        L"STATIC",
        L"NeoWPF Chrome Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        320,
        200,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);

    ASSERT_NE(hwnd, nullptr);

    EXPECT_TRUE(chrome.SetChromeMode(hwnd, NeoWPF::Contracts::ChromeMode::Custom));
    EXPECT_EQ(chrome.GetChromeMode(hwnd), NeoWPF::Contracts::ChromeMode::Custom);

    auto callback = [](void*, int, int) -> NeoWPF::Contracts::ChromeHitTestResult {
        return NeoWPF::Contracts::ChromeHitTestResult::Drag;
    };
    int userData = 42;

    chrome.SetTitleBarHitTest(hwnd, callback, &userData);

    void* storedUserData = nullptr;
    const auto storedCallback = chrome.GetTitleBarHitTest(hwnd, &storedUserData);
    EXPECT_EQ(storedCallback, callback);
    EXPECT_EQ(storedUserData, &userData);

    DestroyWindow(hwnd);
}
#endif

}
