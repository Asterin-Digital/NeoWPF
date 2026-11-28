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
    Runs the application entry point with platform launch data.

    @internal
*/
class EntryRunner final {
public:
    /*
        @summary
        Constructs launch information, invokes the application entry, and shuts down the runtime.

        @param arguments
        Platform arguments, including the executable name on desktop platforms.

        @param dataUri
        Optional launch URI, or a failure to retrieve or convert it.

        @param executableDirectory
        Application executable directory, or a failure to resolve it.

        @returns
        The application exit code, or a non-zero value when entry or shutdown fails.

        @internal
    */
    static int Run(
        NeoWPF::Result<std::vector<std::wstring>> arguments,
        NeoWPF::Result<std::optional<std::string>> dataUri,
        NeoWPF::Result<std::filesystem::path> executableDirectory);
};

}
