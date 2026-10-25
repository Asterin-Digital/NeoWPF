/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <NeoWPF/Core/Application/DeviceCategory.h>
#include <NeoWPF/Contracts/IGeneral.h>
#include <NeoWPF/Contracts/IWindowChrome.h>
#include <NeoWPF/Contracts/IRendering.h>
#include <NeoWPF/Contracts/IWindowing.h>

namespace NeoWPF::Diagnostics {
struct CallStackEntry;
}

namespace NeoWPF::Contracts {

class IPlatform {
public:
    virtual ~IPlatform() = default;

    virtual IClipboard& Clipboard() noexcept = 0;
    virtual IWindowing& Windowing() noexcept = 0;
    virtual IRendering& Rendering() noexcept = 0;
    virtual IWindowChrome* Chrome() noexcept = 0;

    /*
        @summary
        Returns the broad device category hosted by this platform.

        @internal
    */
    virtual NeoWPF::Core::DeviceCategory GetDeviceCategory() const noexcept = 0;

    /*
        @summary
        Runs the application using this platform's lifecycle behavior.

        @param startupTimeout
        Maximum time allowed for platform-specific startup.

        @returns
        The application exit code.

        @internal
    */
    virtual int RunApplication(std::chrono::seconds startupTimeout) = 0;

    virtual bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData) = 0;
    virtual std::filesystem::path GetExecutableDirectory() const = 0;

    virtual bool FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData) = 0;

    virtual NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false) = 0;
    virtual NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters) = 0;

    virtual bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                                       std::vector<std::uint8_t>& outPixels,
                                       int& outWidth, int& outHeight,
                                       int decodeWidth, int decodeHeight) = 0;
    virtual bool DecodeImageFile(const std::wstring& filePath,
                                 std::vector<std::uint8_t>& outPixels,
                                 int& outWidth, int& outHeight,
                                 int decodeWidth, int decodeHeight) = 0;

    virtual bool RegisterFontFile(const std::wstring& filePath) = 0;
    virtual bool RegisterFontMemory(const unsigned char* data, std::size_t size,
                                    const std::wstring& familyName) = 0;
    virtual std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily) = 0;
    virtual int CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                                 int maxEntries, int skipFrames) noexcept = 0;
    virtual void InstallCrashHandler() noexcept = 0;
    virtual void UninstallCrashHandler() noexcept = 0;
    virtual std::wstring CurrentModulePath() const = 0;
    virtual std::uint32_t CurrentThreadId() const noexcept = 0;
};

}
