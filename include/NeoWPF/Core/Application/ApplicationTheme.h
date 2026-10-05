/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <any>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace NeoWPF::Core::ApplicationThemes {

/*
    @summary Identifies a shared color role in an application theme.
*/
enum class PaletteColor {
    Background,
    Surface,
    Foreground,
    MutedForeground,
    Border,
    Accent,
    AccentHover,
    AccentPressed,
    DisabledSurface,
    Focus
};

struct ThemeDefinition {
    std::string Name;
    std::unordered_map<std::string, std::any> Resources;
    std::unordered_map<PaletteColor, std::uint32_t> Palette;
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

    /*
        @summary Retrieves the active color associated with a shared palette role.

        @param paletteColor
        Shared palette role to retrieve.

        @param outColor
        Receives the resolved color.

        @returns True when the active theme defines the requested role.

        @internal
    */
    bool TryGetPaletteColor(PaletteColor paletteColor, NeoWPF::UI::Color& outColor) const;

    const std::string& GetActiveThemeName() const noexcept;
    const ThemeDefinition* GetActiveTheme() const noexcept;

    void Clear();

private:
    ThemeManager() = default;

    std::unordered_map<std::string, ThemeDefinition> m_themes;
    std::unordered_map<PaletteColor, std::uint32_t> m_paletteColors;
    std::string m_activeThemeName;
};

}
