#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/UI/Application.h>
#include <NeoPM/Utils/Logger.h>
#include <Winux/Winux.h>

int main(int argc, char* argv[]) {
    const auto platform = Winux::Platform::Initialize();
    if (platform.Failed()) {
        NeoPM::Utils::Logger::Instance().Error("NeoPM", "Failed to initialize the Winux platform");
        return 1;
    }

    if (argc == 1) {
        return NeoPM::UI::Run();
    }

    auto& logger = NeoPM::Utils::Logger::Instance();
    auto& parser = NeoPM::Core::CommandParser::Instance();

    auto command = parser.Parse(argc, argv);
    if (!command) {
        return 1;
    }

    return command->Execute();
}
