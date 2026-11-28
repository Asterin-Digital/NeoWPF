#include <NeoWPF/NeoWPF.h>

#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/UI/Application.h>
#include <NeoPM/Utils/Logger.h>
#include <Winux/Winux.h>

int NeoWPF::Core::Entry(const NeoWPF::Core::LaunchContext& launch) {
    auto& logger = NeoPM::Utils::Logger::Instance();
    const auto platform = Winux::Platform::Initialize();
    if (platform.Failed()) {
        logger.Error("NeoPM", "Failed to initialize the Winux platform");
        return 1;
    }

    const auto& arguments = launch.Arguments();
    if (arguments.Fail()) {
        logger.Error(
            "NeoPM",
            "Failed to read application arguments: " + arguments.Error());
        return 1;
    }

    if (arguments.Value().size() <= 1) {
        return NeoPM::UI::Run();
    }

    auto& parser = NeoPM::Core::CommandParser::Instance();
    auto command = parser.Parse(arguments.Value());
    if (!command) {
        return 1;
    }

    return command->Execute();
}
