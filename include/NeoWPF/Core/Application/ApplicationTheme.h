/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <any>
#include <string>
#include <unordered_map>

namespace NeoWPF::Core::ApplicationThemes {

struct ThemeDefinition {
    std::string Name;
    std::unordered_map<std::string, std::any> Resources;
};

class ThemeManager {
public:
    static ThemeManager& Instance();

    void RegisterBuiltInThemes();
    void RegisterTheme(const ThemeDefinition& theme);
    bool HasTheme(const std::string& name) const;

    bool ApplyTheme(const std::string& name);
    bool ApplyTheme(const ThemeDefinition& theme);
    bool SetActiveTheme(const std::string& name);

    const std::string& GetActiveThemeName() const noexcept;
    const ThemeDefinition* GetActiveTheme() const noexcept;

    void Clear();

private:
    ThemeManager() = default;

    std::unordered_map<std::string, ThemeDefinition> m_themes;
    std::string m_activeThemeName;
};

}
