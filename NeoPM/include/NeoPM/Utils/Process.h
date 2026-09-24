/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <filesystem>
#include <string>

namespace NeoPM::Utils {
    class Process {
    public:
        static bool StartDetached(const std::filesystem::path& executable,
                                  const std::string& argument);
        static bool IsRunning(const std::string& executableName);
        static bool IsAnotherProcessRunning(const std::string& executableName);
        static bool TerminateByName(const std::string& executableName);
    };
}