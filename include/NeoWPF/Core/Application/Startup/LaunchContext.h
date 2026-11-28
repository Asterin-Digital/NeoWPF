/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <NeoWPF/Feedback/Result.h>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace NeoWPF::Core {

/*
    @summary
    Provides the launch information passed to the application entry function.
*/
class LaunchContext final {
public:
    /*
        @summary
        Returns the command-line arguments supplied to the application.

        @returns
        The stored result, or an error if argument retrieval or conversion failed.
    */
    [[nodiscard]] const NeoWPF::Result<std::vector<std::wstring>>& Arguments() const noexcept;

    /*
        @summary
        Returns the optional launch data URI.

        @returns
        The stored result. A successful empty optional means that no URI was supplied.
    */
    [[nodiscard]] const NeoWPF::Result<std::optional<std::string>>& DataUri() const noexcept;

    /*
        @summary
        Returns the directory containing the running application executable.

        @returns
        The stored result, or an error if the directory could not be resolved.
    */
    [[nodiscard]] const NeoWPF::Result<std::filesystem::path>& ExecutableDirectory() const noexcept;

private:
    friend class EntryRunner;

    LaunchContext(
        NeoWPF::Result<std::vector<std::wstring>> arguments,
        NeoWPF::Result<std::optional<std::string>> dataUri,
        NeoWPF::Result<std::filesystem::path> executableDirectory);

    NeoWPF::Result<std::vector<std::wstring>> m_arguments;
    NeoWPF::Result<std::optional<std::string>> m_dataUri;
    NeoWPF::Result<std::filesystem::path> m_executableDirectory;
};

}
