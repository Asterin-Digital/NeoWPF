#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <unistd.h>

#include <vector>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path GetExecutableDirectory()
{
    std::vector<char> buffer(512);
    while (true)
    {
        const auto length = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (length <= 0)
            return {};

        if (static_cast<std::size_t>(length) < buffer.size())
            return std::filesystem::path(std::string_view(buffer.data(), static_cast<std::size_t>(length))).parent_path();

        buffer.resize(buffer.size() * 2);
    }
}

}