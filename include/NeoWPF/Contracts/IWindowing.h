#pragma once

#include <chrono>
#include <memory>
#include <string>

namespace NeoWPF::Contracts {

class IWindowing {
public:
    virtual ~IWindowing() = default;

    virtual void* CreateWindow(const wchar_t* title, int width, int height) = 0;
    virtual bool IsWindowAlive(void* nativeWindowHandle) const noexcept = 0;
    virtual void GetWindowSize(void* nativeWindowHandle, unsigned int& outWidth, unsigned int& outHeight) const noexcept = 0;
    virtual void ResizeWindow(void* nativeWindowHandle, int width, int height) noexcept = 0;
    virtual void ShowWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void HideWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void CloseWindow(void* nativeWindowHandle) noexcept = 0;
    virtual void InvalidateWindow(void* nativeWindowHandle) noexcept = 0;
    virtual bool PumpMessages(std::chrono::milliseconds timeout) = 0;
    virtual int ExitCode() const noexcept = 0;
    virtual void RequestQuit(int exitCode) noexcept = 0;
    virtual void ShowFatalError(const std::wstring& title, const std::wstring& message) noexcept = 0;
    virtual void SetConsoleVisible(bool visible) noexcept = 0;
    virtual bool IsConsoleVisible() const noexcept = 0;
};

}
