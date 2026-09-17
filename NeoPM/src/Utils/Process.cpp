#include <NeoPM/Utils/Process.h>
#include <NeoPM/Utils/TextEncoding.h>

#include <boost/process/v1.hpp>
#if defined(_WIN32)
#include <boost/process/v1/windows.hpp>
#endif

#include <algorithm>
#include <cctype>

#if defined(_WIN32)
#include <Windows.h>
#include <TlHelp32.h>
#endif

#if defined(__linux__)
#include <csignal>
#include <dirent.h>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace NeoPM::Utils {
    bool Process::StartDetached(const std::filesystem::path& executable,
                                const std::string& argument) {
        try {
#if defined(_WIN32)
            boost::process::v1::child child(
                executable.string(),
                argument,
                boost::process::v1::windows::create_no_window);
#else
            boost::process::v1::child child(executable.string(), argument);
#endif
            child.detach();
            return true;
        }
        catch (const boost::process::v1::process_error&) {
            return false;
        }
    }

    bool Process::IsRunning(const std::string& executableName) {
#if defined(_WIN32)
        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        PROCESSENTRY32W process{};
        process.dwSize = sizeof(process);
        const auto expectedName = Utf8ToWide(executableName);
        auto found = Process32FirstW(snapshot, &process) != FALSE;
        while (found) {
            if (expectedName == process.szExeFile) {
                CloseHandle(snapshot);
                return true;
            }
            found = Process32NextW(snapshot, &process) != FALSE;
        }
        CloseHandle(snapshot);
        return false;
#elif defined(__linux__)
        for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
            const auto processId = entry.path().filename().string();
            if (!entry.is_directory() || processId.empty() ||
                !std::all_of(processId.begin(), processId.end(), [](unsigned char character) {
                    return std::isdigit(character) != 0;
                })) continue;
            std::ifstream comm(entry.path() / "comm");
            std::string name;
            if (comm && std::getline(comm, name) && name == executableName) return true;
        }
        return false;
#else
        return false;
#endif
    }

    bool Process::IsAnotherProcessRunning(const std::string& executableName) {
#if defined(_WIN32)
        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        PROCESSENTRY32W process{};
        process.dwSize = sizeof(process);
        const auto expectedName = Utf8ToWide(executableName);
        const auto currentProcessId = GetCurrentProcessId();
        auto found = Process32FirstW(snapshot, &process) != FALSE;
        while (found) {
            if (process.th32ProcessID != currentProcessId &&
                process.th32ParentProcessID != currentProcessId &&
                expectedName == process.szExeFile) {
                CloseHandle(snapshot);
                return true;
            }
            found = Process32NextW(snapshot, &process) != FALSE;
        }
        CloseHandle(snapshot);
        return false;
#elif defined(__linux__)
        const auto currentProcessId = getpid();
        for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
            const auto processId = entry.path().filename().string();
            if (!entry.is_directory() || processId.empty() ||
                !std::all_of(processId.begin(), processId.end(), [](unsigned char character) {
                    return std::isdigit(character) != 0;
                })) continue;
            const auto pid = static_cast<pid_t>(std::stoi(processId));
            if (pid == currentProcessId) continue;
            std::ifstream comm(entry.path() / "comm");
            std::string name;
            if (comm && std::getline(comm, name) && name == executableName) return true;
        }
        return false;
#else
        return false;
#endif
    }

    bool Process::TerminateByName(const std::string& executableName) {
#if defined(_WIN32)
        const auto currentProcessId = GetCurrentProcessId();
        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        PROCESSENTRY32W process{};
        process.dwSize = sizeof(process);
        const auto expectedName = Utf8ToWide(executableName);
        bool terminated = false;
        if (Process32FirstW(snapshot, &process)) {
            do {
                if (process.th32ProcessID == currentProcessId || expectedName != process.szExeFile) continue;
                const auto handle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, process.th32ProcessID);
                if (handle) {
                    TerminateProcess(handle, 0);
                    WaitForSingleObject(handle, 5000);
                    CloseHandle(handle);
                    terminated = true;
                }
            } while (Process32NextW(snapshot, &process));
        }
        CloseHandle(snapshot);
        return terminated;
#elif defined(__linux__)
        bool terminated = false;
        for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
            const auto processId = entry.path().filename().string();
            if (!entry.is_directory() || processId.empty() ||
                !std::all_of(processId.begin(), processId.end(), [](unsigned char character) {
                    return std::isdigit(character) != 0;
                })) continue;
            std::ifstream comm(entry.path() / "comm");
            std::string name;
            if (comm && std::getline(comm, name) && name == executableName) {
                const auto pid = static_cast<pid_t>(std::stoi(entry.path().filename().string()));
                if (pid != getpid() && kill(pid, SIGTERM) == 0) terminated = true;
            }
        }
        return terminated;
#else
        return false;
#endif
    }
}