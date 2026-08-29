#include <NeoWPF/Diagnostics/CrashHandler.h>

#include <cerrno>
#include <cstdint>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

namespace NeoWPF::Platform::Linux {

void InstallCrashHandlerLinux() noexcept
{
}

void UninstallCrashHandlerLinux() noexcept
{
}

std::wstring CurrentModulePathLinux()
{
    char path[4096] = {};
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length <= 0) {
        return L"<unknown>";
    }
    path[length] = '\0';

    std::wstring result;
    result.reserve(static_cast<std::size_t>(length));
    for (ssize_t index = 0; index < length; ++index) {
        const unsigned char character = static_cast<unsigned char>(path[index]);
        result.push_back(character < 0x80 ? static_cast<wchar_t>(character) : L'?');
    }
    return result;
}

std::uint32_t CurrentThreadIdLinux() noexcept
{
    return static_cast<std::uint32_t>(syscall(SYS_gettid));
}

}