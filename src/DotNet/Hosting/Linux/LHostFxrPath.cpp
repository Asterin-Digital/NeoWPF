#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path ResolvePath(const HostOptions& options)
{
    const auto root = std::filesystem::weakly_canonical(options.dotnetRoot);
    return options.hostfxrPath.empty() ? root / "libhostfxr.so" : options.hostfxrPath;
}

}
