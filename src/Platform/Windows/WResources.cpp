#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>

#include <NeoWPF/Etc/Result.h>
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

std::filesystem::path GetExecutableDirectory() {
    wchar_t path[MAX_PATH] = {};
    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(path).parent_path();
}

}
