/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostStatus.h>

#include <filesystem>
#include <functional>
#include <string>

namespace NeoWPF::DotNet::Hosting {

struct HostOptions {
    std::filesystem::path hostfxrPath;
    std::filesystem::path dotnetRoot;
    std::filesystem::path bridgeDirectory;
    std::function<void(std::uint32_t, const std::string&, const std::string&)> log;
};

class Host;

}
