#include <NeoPM/Core/Daemon/StartupTaskInstaller.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Utils/FileSystem.h>
#include <NeoPM/Utils/Logger.h>
#include <NeoPM/Utils/Startup.h>

namespace NeoPM::Core::Daemon {
    namespace {
    }

    bool StartupTaskInstaller::InstallUserStartupEntry() {
        auto executablePath = ToolchainBootstrap::GetToolExecutablePath();
        const auto configPath = Utils::FileSystem::GetUserConfigPath();
        if (!configPath || !Utils::Startup::InstallUserEntry(executablePath, *configPath)) {
            Utils::Logger::Instance().Error("StartupTaskInstaller", "Failed to register NeoPM startup entry");
            return false;
        }

        Utils::Logger::Instance().Info("StartupTaskInstaller", "Registered NeoPM to start at user logon");
        return true;
    }
}