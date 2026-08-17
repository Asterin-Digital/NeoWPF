#include <NeoPM/Core/CommandParser.h>
#include <NeoPM/Utils/Logger.h>

int main(int argc, char* argv[]) {
    auto& logger = NeoPM::Utils::Logger::Instance();
    auto& parser = NeoPM::Core::CommandParser::Instance();

    auto command = parser.Parse(argc, argv);
    if (!command) {
        return 1;
    }

    return command->Execute();
}
