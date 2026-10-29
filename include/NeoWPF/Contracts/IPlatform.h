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

/*
    @summary
    Provides the platform services used by the application runtime.
*/
class IPlatform {
public:
    virtual ~IPlatform() = default;

    /*
        @summary
        Returns the platform clipboard service.
    */
    virtual IClipboard& Clipboard() noexcept = 0;
    /*
        @summary
        Returns the platform windowing service.
    */
    virtual IWindowing& Windowing() noexcept = 0;
    /*
        @summary
        Returns the platform rendering service.
    */
    virtual IRendering& Rendering() noexcept = 0;
    /*
        @summary
        Returns the optional native window chrome service.
    */
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

    /*
        @summary
        Loads resource bytes from the platform's resource store.

        @param resourcePath
        Resource path to load.

        @param outData
        Receives the loaded bytes.

        @returns
        True when the resource was loaded.
    */
    virtual bool LoadResourceData(const std::wstring& resourcePath, std::vector<std::uint8_t>& outData) = 0;
    /*
        @summary
        Returns the directory containing the application executable.
    */
    virtual std::filesystem::path GetExecutableDirectory() const = 0;

    /*
        @summary
        Downloads a resource into a byte buffer.

        @param url
        Resource URL.

        @param outData
        Receives the downloaded bytes.

        @returns
        True when the resource was retrieved.
    */
    virtual bool FetchHttpResource(const std::wstring& url, std::vector<std::uint8_t>& outData) = 0;

    /*
        @summary
        Displays a file-open dialog.

        @param title
        Dialog title.

        @param initialDirectory
        Directory shown when the dialog opens.

        @param filters
        File types offered by the dialog.

        @param allowMultiSelect
        Whether multiple files may be selected.
    */
    virtual NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false) = 0;
    /*
        @summary
        Displays a file-save dialog.

        @param title
        Dialog title.

        @param initialDirectory
        Directory shown when the dialog opens.

        @param defaultFileName
        Initial file name.

        @param filters
        File types offered by the dialog.
    */
    virtual NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters) = 0;

    /*
        @summary
        Decodes image pixels from encoded memory data.

        @param data
        Encoded image bytes.

        @param outPixels
        Receives decoded pixel data.

        @param outWidth
        Receives the decoded image width.

        @param outHeight
        Receives the decoded image height.

        @param decodeWidth
        Requested output width.

        @param decodeHeight
        Requested output height.

        @returns
        True when decoding succeeds.
    */
    virtual bool DecodeImageFromMemory(const std::vector<std::uint8_t>& data,
                                       std::vector<std::uint8_t>& outPixels,
                                       int& outWidth, int& outHeight,
                                       int decodeWidth, int decodeHeight) = 0;
    /*
        @summary
        Decodes image pixels from a file.

        @param filePath
        Path to the encoded image.

        @param outPixels
        Receives decoded pixel data.

        @param outWidth
        Receives the decoded image width.

        @param outHeight
        Receives the decoded image height.

        @param decodeWidth
        Requested output width.

        @param decodeHeight
        Requested output height.

        @returns
        True when decoding succeeds.
    */
    virtual bool DecodeImageFile(const std::wstring& filePath,
                                 std::vector<std::uint8_t>& outPixels,
                                 int& outWidth, int& outHeight,
                                 int decodeWidth, int decodeHeight) = 0;

    /*
        @summary
        Registers a font from a file.

        @param filePath
        Path to the font file.

        @returns
        True when the font is registered.
    */
    virtual bool RegisterFontFile(const std::wstring& filePath) = 0;
    /*
        @summary
        Registers font data from memory.

        @param data
        Font data buffer.

        @param size
        Number of bytes in the buffer.

        @param familyName
        Font family name.

        @returns
        True when the font is registered.
    */
    virtual bool RegisterFontMemory(const unsigned char* data, std::size_t size,
                                    const std::wstring& familyName) = 0;
    /*
        @summary
        Resolves a requested font family to an available family.

        @param requestedFontFamily
        Requested family name.
    */
    virtual std::wstring ResolveFontFamily(const std::wstring& requestedFontFamily) = 0;
    /*
        @summary
        Captures the current call stack into the supplied buffer.

        @param entries
        Receives captured stack entries.

        @param maxEntries
        Maximum number of entries to capture.

        @param skipFrames
        Number of leading frames to omit.

        @returns
        Number of entries captured.
    */
    virtual int CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry* entries,
                                 int maxEntries, int skipFrames) noexcept = 0;
    /*
        @summary
        Installs the platform crash handler.
    */
    virtual void InstallCrashHandler() noexcept = 0;
    /*
        @summary
        Removes the platform crash handler.
    */
    virtual void UninstallCrashHandler() noexcept = 0;
    /*
        @summary
        Returns the path of the current module.
    */
    virtual std::wstring CurrentModulePath() const = 0;
    /*
        @summary
        Returns the identifier of the current thread.
    */
    virtual std::uint32_t CurrentThreadId() const noexcept = 0;
};

}
