#include <NeoWPF/Platform/Resources.h>
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>

namespace NeoWPF::Platform {

bool LoadResourceData(const std::wstring& resourcePath, std::vector<uint8_t>& outData) {
    HRSRC hResource = FindResourceW(nullptr, resourcePath.c_str(), RT_RCDATA);
    if (!hResource) return false;

    HGLOBAL hGlobal = LoadResource(nullptr, hResource);
    if (!hGlobal) return false;

    LPVOID pData = LockResource(hGlobal);
    if (!pData) return false;

    DWORD dwSize = SizeofResource(nullptr, hResource);
    if (dwSize == 0) return false;

    outData.assign(static_cast<uint8_t*>(pData), static_cast<uint8_t*>(pData) + dwSize);
    return true;
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
