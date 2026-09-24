/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Core/ICommand.h>
#include <filesystem>
#include <string>
#include <optional>

namespace NeoPM::Core::Commands {
    class SyncResourcesCommand : public ICommand {
    public:
        explicit SyncResourcesCommand(
            const std::string& manifestPath,
            const std::optional<std::string>& outputDir = std::nullopt,
            const std::optional<std::string>& projectName = std::nullopt,
            const std::optional<std::string>& copyOutputDir = std::nullopt
        );

        int Execute() override;
        const char* GetName() const override;
        const char* GetDescription() const override;

    private:
        std::string m_manifestPath;
        std::optional<std::string> m_outputDir;
        std::optional<std::string> m_projectName;
        std::optional<std::string> m_copyOutputDir;

        // Resolve project name from manifest path if not provided
        std::string ResolveProjectName(const std::filesystem::path& manifestPath) const;
    };
}
