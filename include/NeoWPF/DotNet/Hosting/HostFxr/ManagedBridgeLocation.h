/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

/*
    @summary
    Gets the directory containing the current executable.

    @returns
    The executable's parent directory, or an empty path on failure.

    @internal
*/
std::filesystem::path GetExecutableDirectory();

std::filesystem::path ResolveLocation(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& configuredDirectory = {});

}
