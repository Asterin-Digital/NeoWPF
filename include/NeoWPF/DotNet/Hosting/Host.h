/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostOptions.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting {

class Host {
public:
    HostStatus Start(const HostOptions& options = {});
    bool Stop();

    static std::filesystem::path ResolveHostFxrPath(const HostOptions& options);
    static HostStatus ResolveHostFxrPathStatus(const HostOptions& options);
    static std::filesystem::path ResolveManagedBridgeDirectory(
        const std::filesystem::path& executableDirectory,
        const std::filesystem::path& bridgeDirectory = {});

private:
    bool m_started = false;
};

}
