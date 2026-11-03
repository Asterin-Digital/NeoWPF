/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IWindowing.h>
#include <cstdint>

#ifdef CreateWindow
#undef CreateWindow
#endif
#ifdef CreateWindowEx
#undef CreateWindowEx
#endif
#ifdef CreateWindowExA
#undef CreateWindowExA
#endif
#ifdef CreateWindowExW
#undef CreateWindowExW
#endif

namespace NeoWPF::Platform::Windows {

/*
    @summary
    Converts a Windows virtual-key code to a NeoWPF key code.

    @param virtualKey
    Windows virtual-key code.

    @returns
    The matching key code, or zero when it is not mapped.

    @internal
*/
int TranslateVirtualKey(std::uint32_t virtualKey) noexcept;

class WWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    void* CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        std::shared_ptr<NeoWPF::UI::Window> windowHost = nullptr) override;
    bool GetNativeWindowInfo(void* nativeWindowHandle, NeoWPF::Contracts::NativeWindowInfo& outInfo) const noexcept override;
    bool IsWindowAlive(void* nativeWindowHandle) const noexcept override;
    void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept override;
    void ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept override;
    void ShowWindow(void* nativeWindowHandle) noexcept override;
    void HideWindow(void* nativeWindowHandle) noexcept override;
    void CloseWindow(void* nativeWindowHandle) noexcept override;
    void InvalidateWindow(void* nativeWindowHandle) noexcept override;
    bool PumpMessages(std::chrono::milliseconds timeout) override;
    int ExitCode() const noexcept override;
    void RequestQuit(int exitCode) noexcept override;
    void ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept override;
    void SetConsoleVisible(bool visible) noexcept override;
    bool IsConsoleVisible() const noexcept override;

private:
    int m_exitCode = 0;
};

}
