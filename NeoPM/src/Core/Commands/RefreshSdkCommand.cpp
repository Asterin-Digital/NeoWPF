#include <NeoPM/Core/Commands/RefreshSdkCommand.h>
#include <NeoPM/Sdk/SdkManager.h>
#include <NeoPM/Utils/Logger.h>

#include <iostream>

namespace NeoPM::Core::Commands {

RefreshSdkCommand::RefreshSdkCommand(const std::string& channel)
    : m_channel(channel) {
}

int RefreshSdkCommand::Execute() {
    auto& logger = Utils::Logger::Instance();

    logger.Info("RefreshSdkCommand", "Refreshing NeoWPF SDK...");

    SdkManager sdkManager;
    if (!sdkManager.RefreshSdks()) {
        logger.Error("RefreshSdkCommand", "Failed to refresh SDKs");
        std::cerr << "Failed to refresh SDK" << std::endl;
        return 1;
    }


    auto debugPath = sdkManager.GetSdkPath("debug");
    auto releasePath = sdkManager.GetSdkPath("release");

    if (debugPath) {
        std::cout << "Debug SDK installed: " << debugPath.value().string() << std::endl;
    }

    if (releasePath) {
        std::cout << "Release SDK installed: " << releasePath.value().string() << std::endl;
    }

    if (!debugPath && !releasePath) {
        logger.Warning("RefreshSdkCommand", "No SDKs were installed");
    }

    logger.Info("RefreshSdkCommand", "SDK refresh complete");
    return 0;
}

const char* RefreshSdkCommand::GetName() const {
    return "refresh-sdk";
}

const char* RefreshSdkCommand::GetDescription() const {
    return "Download and install latest NeoWPF SDKs from GitHub";
}

}
