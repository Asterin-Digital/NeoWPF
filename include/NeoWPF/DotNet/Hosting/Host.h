/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/DotNet/Hosting/HostOptions.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrLibrary.h>
#include <NeoWPF/DotNet/Hosting/HostSession.h>
#include <NeoWPF/DotNet/Abi/NwBridgeApi.h>

#include <filesystem>
#include <functional>
#include <string>

namespace NeoWPF::DotNet::Hosting {

class Host {
public:
    HostStatus Start(const HostOptions& options = {});
    bool Stop();

    const HostSession& GetSession() const noexcept;

    static std::filesystem::path ResolveHostFxrPath(const HostOptions& options);
    static HostStatus ResolveHostFxrPathStatus(const HostOptions& options);
    static std::filesystem::path ResolveManagedBridgeDirectory(
        const std::filesystem::path& executableDirectory,
        const std::filesystem::path& bridgeDirectory = {});

private:
    HostSession m_session;
    HostFxr::HostFxrLibrary m_hostFxrLibrary;
    void* m_runtimeDelegate = nullptr;
    std::function<void(std::uint32_t, const std::string&, const std::string&)> m_log;
};

}
