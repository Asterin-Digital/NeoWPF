#include <NeoWPF/DotNet/Hosting/Host.h>

#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

namespace NeoWPF::DotNet::Hosting {

HostStatus Host::Start(const HostOptions& options)
{
    if (m_started)
        return HostStatus(HostStatusCode::AlreadyStarted);

    const auto hostStatus = ResolveHostFxrPathStatus(options);
    if (!hostStatus.Succeeded())
        return hostStatus;

    return HostStatus(HostStatusCode::HostFxrLoadFailed)
        .SetMessage("Hostfxr was located, but its loader is unavailable.")
        .SetHint("Complete hostfxr loading and bridge initialization before starting the host.");
}

bool Host::Stop()
{
    if (!m_started)
        return false;

    m_started = false;
    return true;
}

std::filesystem::path Host::ResolveHostFxrPath(const HostOptions& options)
{
    return HostFxr::ResolvePath(options);
}

HostStatus Host::ResolveHostFxrPathStatus(const HostOptions& options)
{
    return HostFxr::ResolveStatus(options);
}

std::filesystem::path Host::ResolveManagedBridgeDirectory(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& bridgeDirectory)
{
    return HostFxr::ResolveLocation(executableDirectory, bridgeDirectory);
}

}
