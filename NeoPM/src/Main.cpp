#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/UI/Application.h>
#include <NeoPM/Utils/Logger.h>

int main(int argc, char* argv[]) {
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
