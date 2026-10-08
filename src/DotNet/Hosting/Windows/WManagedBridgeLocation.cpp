#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <Windows.h>

#include <vector>

namespace NeoWPF::DotNet::Hosting::HostFxr {

std::filesystem::path GetExecutableDirectory()
{
    std::vector<wchar_t> buffer(512);
    while (true)
    {
        const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
            return {};

        if (length < buffer.size())
            return std::filesystem::path(std::wstring_view(buffer.data(), length)).parent_path();

        buffer.resize(buffer.size() * 2);
    }
}

}