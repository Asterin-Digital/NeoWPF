#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path ResolveLocation(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& configuredDirectory)
{
    const std::filesystem::path configured = configuredDirectory.empty()
        ? executableDirectory / "NeoWPF.DotNet"
        : configuredDirectory;

    return configured;
}

}
