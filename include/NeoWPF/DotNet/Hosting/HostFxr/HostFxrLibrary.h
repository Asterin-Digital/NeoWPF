/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostOptions.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

class HostFxrLibrary final {
public:
    /*
        @summary
        Loads hostfxr for the process lifetime.

        @param path
        Absolute path to the hostfxr dynamic library.

        @returns
        Status describing whether the library loaded successfully.

        @internal
    */
    HostStatus Load(const std::filesystem::path& path);

    /*
        @summary
        Gets an exported hostfxr function address.

        @param name
        Null-terminated export name.

        @returns
        The export address, or null when it is unavailable.

        @internal
    */
    void* GetExport(const char* name) const noexcept;

    /*
        @summary
        Captures hostfxr diagnostics on the current thread.

        @param setErrorWriter
        Optional hostfxr_set_error_writer export address.

        @internal
    */
    void SetErrorWriter(void* setErrorWriter) noexcept;

    /*
        @summary
        Restores the previous hostfxr error writer for the current thread.

        @internal
    */
    void ClearErrorWriter() noexcept;

    /*
        @summary
        Gets diagnostics captured from hostfxr on the current thread.

        @returns
        The captured diagnostic text.

        @internal
    */
    const std::string& GetErrorMessage() const noexcept;

    /*
        @summary
        Checks whether hostfxr exports a symbol.

        @param name
        Null-terminated export name.

        @returns
        True when the loaded library exports the name.

        @internal
    */
    bool HasExport(const char* name) const noexcept;

private:
    void* m_handle = nullptr;
};

}