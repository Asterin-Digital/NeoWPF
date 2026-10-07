/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path ResolveLocation(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& configuredDirectory = {});

}
