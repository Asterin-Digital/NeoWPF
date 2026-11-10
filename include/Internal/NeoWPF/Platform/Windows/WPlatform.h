/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IPlatform.h>
#include <NeoWPF/Platform/CurlHttp.h>
#include <NeoWPF/Platform/Windows/WClipboard.h>
#include <NeoWPF/Platform/Windows/WFileDialog.h>
#include <NeoWPF/Platform/Windows/WWindowChrome.h>
#include <NeoWPF/Platform/Windows/WWindowing.h>
#include <NeoWPF/Rendering/D3D11/WRendering.h>

namespace NeoWPF::Platform::Windows {

class WPlatform final : public NeoWPF::Contracts::IPlatform {
public:
    WPlatform();

    /*
        @summary
        Returns the Windows clipboard service.

        @internal
    */
    NeoWPF::Contracts::IClipboard& Clipboard() noexcept override;
    /*
        @summary
        Returns the Windows windowing service.

        @internal
    */
    NeoWPF::Contracts::IWindowing& Windowing() noexcept override;
    /*
        @summary
        Returns the Windows rendering service.

        @internal
    */
    NeoWPF::Contracts::IRendering& Rendering() noexcept override;
    /*
        @summary
        Returns the Windows window chrome service.

        @internal
    */
    NeoWPF::Contracts::IWindowChrome* Chrome() noexcept override;
    /*
        @summary
        Returns the Windows platform identifier.

        @internal
    */
    NeoWPF::Platform::Capabilities::PlatformId GetPlatformId() const noexcept override;
    /*
        @summary
        Returns the desktop device category.

        @internal
    */
    NeoWPF::Core::DeviceCategory GetDeviceCategory() const noexcept override;
    /*
        @summary
        Runs the Windows application lifecycle.

        @param startupTimeout
        Maximum platform startup duration.

        @returns
        The application exit code.

        @internal
    */
    int RunApplication(std::chrono::seconds startupTimeout) override;

    /*
        @summary
        Loads resource bytes from the Windows resource store.

        @param resourcePath
        Resource path to load.

        @returns
        The loaded bytes, or the failure reason.

        @internal
    */
    NeoWPF::Result<std::vector<std::uint8_t>> LoadResourceData(const std::wstring& resourcePath) override;
    /*
        @summary
        Returns the directory containing the Windows executable.

        @returns
        The executable directory.

        @internal
    */
    std::filesystem::path GetExecutableDirectory() const override;
    /*
        @summary
        Returns the Windows Local AppData directory.

        @internal
    */
    NeoWPF::Result<std::filesystem::path> GetAppDataDirectory() const override;
    /*
        @summary
        Downloads a resource into a byte buffer.

        @param url
        Resource URL.

        @returns
        The downloaded bytes, or the failure reason.

        @internal
    */
    NeoWPF::Result<std::vector<std::uint8_t>> FetchHttpResource(const std::wstring& url) override;
    /*
        @summary
        Displays a Windows file-open dialog.

        @param title
        Dialog title.

        @param initialDirectory
        Directory shown when the dialog opens.

        @param filters
        File types offered by the dialog.

        @param allowMultiSelect
        Whether multiple files may be selected.

        @returns
        The selected files, an empty value when canceled, or the failure reason.

        @internal
    */
    NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false) override;
    /*
        @summary
        Displays a Windows file-save dialog.

        @param title
        Dialog title.

        @param initialDirectory
        Directory shown when the dialog opens.

        @param defaultFileName
        Initial file name.

        @param filters
        File types offered by the dialog.

        @returns
        The selected file, an empty value when canceled, or the failure reason.

        @internal
    */
    NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters) override;
    /*
        @summary
        Decodes image pixels from encoded memory data.

        @param data
        Encoded image bytes.

        @param decodeWidth
        Requested output width.

        @param decodeHeight
        Requested output height.

        @returns
        The decoded image pixels and dimensions, or the failure reason.

        @internal
    */
    NeoWPF::Result<NeoWPF::Contracts::DecodedImage> DecodeImageFromMemory(
        const std::vector<std::uint8_t>& data, int decodeWidth, int decodeHeight) override;
    /*
        @summary
        Decodes image pixels from a file.

        @param filePath
        Path to the encoded image.

        @param decodeWidth
        Requested output width.

        @param decodeHeight
        Requested output height.

        @returns
        The decoded image pixels and dimensions, or the failure reason.

        @internal
    */
    NeoWPF::Result<NeoWPF::Contracts::DecodedImage> DecodeImageFile(
        const std::wstring& filePath, int decodeWidth, int decodeHeight) override;
    /*
        @summary
        Registers a font from a file.

        @param filePath
        Font file path.

        @returns
        Success when the font was registered, or the failure reason.

        @internal
    */
    NeoWPF::Result<void> RegisterFontFile(const std::wstring& filePath) override;
    /*
        @summary
        Registers font data.

        @param data
        Font data bytes.

        @param size
        Number of bytes in the font data.

        @param familyName
        Font family name.

        @returns
        Success when the font was registered, or the failure reason.

        @internal
    */
    NeoWPF::Result<void> RegisterFontMemory(
        const unsigned char* data, std::size_t size, const std::wstring& familyName) override;
    /*
        @summary
        Resolves a requested font family.

        @param requestedFontFamily
        Requested font family.

        @returns
        The resolved font family.

        @internal
    */
    std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily) override;
    /*
        @summary
        Captures the current call stack.

        @param entries
        Receives captured entries.

        @param maxEntries
        Maximum number of entries to capture.

        @param skipFrames
        Number of initial frames to skip.

        @returns
        The number of captured entries.

        @internal
    */
    int CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                         int maxEntries, int skipFrames) noexcept override;
    /*
        @summary
        Installs the Windows crash handler.

        @internal
    */
    void InstallCrashHandler() noexcept override;
    /*
        @summary
        Removes the Windows crash handler.

        @internal
    */
    void UninstallCrashHandler() noexcept override;
    /*
        @summary
        Returns the path of the current module.

        @returns
        The current module path.

        @internal
    */
    std::wstring CurrentModulePath() const override;
    /*
        @summary
        Returns the current thread identifier.

        @returns
        The current thread identifier.

        @internal
    */
    std::uint32_t CurrentThreadId() const noexcept override;

private:
    NeoWPF::Platform::CurlHttp m_http;
    WClipboard m_clipboard;
    WWindowing m_windowing;
    WWindowChrome m_chrome;
    NeoWPF::Rendering::D3D11::WRendering m_rendering;
};

}
