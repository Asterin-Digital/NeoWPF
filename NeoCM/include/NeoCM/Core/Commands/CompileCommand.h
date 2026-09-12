#pragma once

#include <NeoCM/Core/ICommand.h>
#include <string>
#include <optional>

namespace NeoCM::Core::Commands {

struct CompileArguments {
    std::string inputPath;
    std::optional<std::string> outputPath;
    bool showHelp = false;
};

class CompileCommand final : public NeoCM::Core::ICommand {
public:
    explicit CompileCommand(CompileArguments arguments);
    int Execute() override;

private:
    CompileArguments m_arguments;

    int CompileFile() const;
    static std::string DefaultOutputPath(const std::string& inputPath);
};

}
