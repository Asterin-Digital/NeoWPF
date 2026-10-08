#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>

#include <nethost.h>

#include <vector>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path ResolvePath(const HostOptions& options)
{
    if (!options.hostfxrPath.empty())
        return options.hostfxrPath;

    std::vector<char_t> buffer(32768);
    std::size_t bufferSize = buffer.size();
    get_hostfxr_parameters parameters{};
    parameters.size = sizeof(parameters);
    parameters.dotnet_root = options.dotnetRoot.empty() ? nullptr : options.dotnetRoot.c_str();

    if (get_hostfxr_path(buffer.data(), &bufferSize, &parameters) != 0)
        return {};

    return std::filesystem::path(buffer.data());
}

}
