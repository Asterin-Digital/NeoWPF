/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Core/ICommand.h>

#include <filesystem>

namespace NeoPM::Core::Commands {

class RepairCommand final : public ICommand {
public:
    explicit RepairCommand(std::filesystem::path projectPath);

    int Execute() override;
    const char* GetName() const override;
    const char* GetDescription() const override;

private:
    std::filesystem::path m_projectPath;
};

}
