#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>
#include <limits>

#include <NeoWPF/Feedback/Result.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoWPF::Platform {

NeoWPF::Result<std::vector<uint8_t>> LoadResourceData(const std::wstring& resourcePath) {
    HRSRC hResource = FindResourceW(nullptr, resourcePath.c_str(), RT_RCDATA);
    if (!hResource) return NeoWPF::Fail<std::string>{"Windows resource was not found"};

    HGLOBAL hGlobal = LoadResource(nullptr, hResource);
    if (!hGlobal) return NeoWPF::Fail<std::string>{"Windows resource could not be loaded"};

    LPVOID pData = LockResource(hGlobal);
    if (!pData) return NeoWPF::Fail<std::string>{"Windows resource data could not be accessed"};

    DWORD dwSize = SizeofResource(nullptr, hResource);
    if (dwSize == 0) return NeoWPF::Fail<std::string>{"Windows resource is empty"};

    return NeoWPF::Success<std::vector<uint8_t>>{
        std::vector<uint8_t>(static_cast<uint8_t*>(pData), static_cast<uint8_t*>(pData) + dwSize)};
}

NeoWPF::Result<std::filesystem::path> GetExecutableDirectory()
{
    std::vector<wchar_t> path(256);
    for (;;) {
        ::SetLastError(ERROR_SUCCESS);
        const DWORD length = ::GetModuleFileNameW(
            nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0) {
            return NeoWPF::Fail<std::string>{
                "Failed to resolve the Windows executable path (error " +
                std::to_string(::GetLastError()) + ")."};
        }
        if (length < path.size()) {
            const auto directory =
                std::filesystem::path(path.data(), path.data() + length).parent_path();
            if (directory.empty()) {
                return NeoWPF::Fail<std::string>{
                    "Windows returned an executable path without a parent directory."};
            }
            return NeoWPF::Success<std::filesystem::path>{directory};
        }
        if (path.size() > (std::numeric_limits<DWORD>::max)() / 2) {
            return NeoWPF::Fail<std::string>{
                "The Windows executable path exceeds the supported buffer size."};
        }
        path.resize(path.size() * 2);
    }
}

}
