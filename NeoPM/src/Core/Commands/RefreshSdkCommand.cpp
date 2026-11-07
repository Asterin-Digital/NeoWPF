#include <NeoPM/Core/Commands/RefreshSdkCommand.h>
#include <NeoPM/Sdk/SdkManager.h>
#include <NeoPM/Utils/Logger.h>

#include <iostream>

namespace NeoPM::Core::Commands {

int RefreshSdkCommand::Execute() {
    auto& logger = Utils::Logger::Instance();

    logger.Info("RefreshSdkCommand", "Refreshing NeoWPF SDK...");

    SdkManager sdkManager;
    if (!sdkManager.RefreshSdks() || !sdkManager.HasLocalSdks()) {
        logger.Error("RefreshSdkCommand", "No complete platform SDK is available");
        std::cerr << "No complete platform SDK is available" << std::endl;
        return 1;
    }

    const auto sdkPath = sdkManager.GetSdkPath();
    std::cout << "Platform SDK installed: " << sdkPath->string() << std::endl;

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
