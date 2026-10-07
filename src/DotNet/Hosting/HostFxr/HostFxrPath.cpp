#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>

#include <filesystem>

namespace NeoWPF::DotNet::Hosting::HostFxr {

HostStatus ResolveStatus(const HostOptions& options)
{
    const auto path = ResolvePath(options);
    if (!path.empty() && std::filesystem::exists(path))
        return HostStatus(HostStatusCode::Ok);

    return HostStatus(HostStatusCode::RuntimeNotFound)
        .SetMessage("No hostfxr runtime path was found.")
        .SetHint("Provide hostfxrPath or dotnetRoot.");
}

}
