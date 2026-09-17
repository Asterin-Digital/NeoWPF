#include <NeoPM/Utils/Startup.h>

#include <fstream>
#include <string>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace NeoPM::Utils {
    bool Startup::InstallUserEntry(const std::filesystem::path& executablePath,
                                   const std::filesystem::path& configPath) {
        std::error_code error;
        std::filesystem::create_directories(configPath, error);
        if (error) return false;

#if defined(_WIN32)
        const auto command = L"\"" + executablePath.wstring() + L"\" daemon";
        HKEY key = nullptr;
        const auto result = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr);
        if (result != ERROR_SUCCESS || key == nullptr) return false;

        const auto valueResult = RegSetValueExW(
            key,
            L"NeoPM",
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(command.c_str()),
            static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(key);
        return valueResult == ERROR_SUCCESS;
#else
        const auto startupPath = configPath / "autostart" / "neopm.desktop";
        std::filesystem::create_directories(startupPath.parent_path(), error);
        if (error) return false;
        std::ofstream startup(startupPath, std::ios::trunc);
        if (!startup) return false;
        startup << "[Desktop Entry]\n"
                << "Type=Application\n"
                << "Name=NeoPM\n"
                << "Exec=\"" << executablePath.string() << "\" daemon\n"
                << "X-GNOME-Autostart-enabled=true\n";
        return static_cast<bool>(startup);
#endif
    }
}