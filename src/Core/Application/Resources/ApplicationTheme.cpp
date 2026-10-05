#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <cstdint>
#include <utility>

namespace NeoWPF::Core::ApplicationThemes {

namespace {

const char* GetPaletteResourceKey(PaletteColor paletteColor) noexcept
{
    switch (paletteColor) {
    case PaletteColor::Background: return "Palette.Background";
    case PaletteColor::Surface: return "Palette.Surface";
    case PaletteColor::Foreground: return "Palette.Foreground";
    case PaletteColor::MutedForeground: return "Palette.MutedForeground";
    case PaletteColor::Border: return "Palette.Border";
    case PaletteColor::Accent: return "Palette.Accent";
    case PaletteColor::AccentHover: return "Palette.AccentHover";
    case PaletteColor::AccentPressed: return "Palette.AccentPressed";
    case PaletteColor::DisabledSurface: return "Palette.DisabledSurface";
    case PaletteColor::Focus: return "Palette.Focus";
    }
    return nullptr;
}

}

ThemeManager& ThemeManager::Instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::RegisterBuiltInThemes()
{
    ThemeDefinition light;
    light.Name = "Light Mint";
    light.Palette[PaletteColor::Background] = 0xFFF1F2EFu;
    light.Palette[PaletteColor::Surface] = 0xFFF8F9F7u;
    light.Palette[PaletteColor::Foreground] = 0xFF252925u;
    light.Palette[PaletteColor::MutedForeground] = 0xFF737A74u;
    light.Palette[PaletteColor::Border] = 0xFF9AA19Bu;
    light.Palette[PaletteColor::Accent] = 0xFF708A74u;
    light.Palette[PaletteColor::AccentHover] = 0xFFE4EBE3u;
    light.Palette[PaletteColor::AccentPressed] = 0xFFD6E1D6u;
    light.Palette[PaletteColor::DisabledSurface] = 0xFFE9ECE9u;
    light.Palette[PaletteColor::Focus] = 0xFF718C76u;

    ThemeDefinition dark;
    dark.Name = "Dark Mint";
    dark.Palette[PaletteColor::Background] = 0xFF131313u;
    dark.Palette[PaletteColor::Surface] = 0xFF1D1D1Du;
    dark.Palette[PaletteColor::Foreground] = 0xFFE6E6E6u;
    dark.Palette[PaletteColor::MutedForeground] = 0xFF9B9B9Bu;
    dark.Palette[PaletteColor::Border] = 0xFF686868u;
    dark.Palette[PaletteColor::Accent] = 0xFF8A9B8Bu;
    dark.Palette[PaletteColor::AccentHover] = 0xFF252B26u;
    dark.Palette[PaletteColor::AccentPressed] = 0xFF303730u;
    dark.Palette[PaletteColor::DisabledSurface] = 0xFF222222u;
    dark.Palette[PaletteColor::Focus] = 0xFF849786u;

    RegisterTheme(light);
    RegisterTheme(dark);
}

void ThemeManager::RegisterTheme(const ThemeDefinition& theme)
{
    if (theme.Name.empty()) {
        return;
    }

    m_themes[theme.Name] = theme;
}

bool ThemeManager::HasTheme(const std::string& name) const
{
    return m_themes.find(name) != m_themes.end();
}

bool ThemeManager::ApplyTheme(const std::string& name)
{
    const auto it = m_themes.find(name);
    if (it == m_themes.end()) {
        return false;
    }

    return ApplyTheme(it->second);
}

bool ThemeManager::ApplyTheme(const ThemeDefinition& theme)
{
    if (theme.Name.empty()) {
        return false;
    }

    for (const auto& [paletteColor, value] : theme.Palette) {
        const auto* key = GetPaletteResourceKey(paletteColor);
        if (!key) {
            continue;
        }
        NeoWPF::Core::ApplicationResources::Resources::SetResource(key, value);
        m_paletteColors[paletteColor] = value;
    }

    for (const auto& [key, value] : theme.Resources) {
        NeoWPF::Core::ApplicationResources::Resources::SetResource(key, value);
    }

    NeoWPF::Diagnostics::LogInfo(("Applied application theme: " + theme.Name).c_str());
    return true;
}

bool ThemeManager::SetActiveTheme(const std::string& name)
{
    if (!ApplyTheme(name)) {
        return false;
    }

    m_activeThemeName = name;
    return true;
}

bool ThemeManager::TryGetPaletteColor(PaletteColor paletteColor, NeoWPF::UI::Color& outColor) const
{
    const auto it = m_paletteColors.find(paletteColor);
    if (it == m_paletteColors.end()) {
        return false;
    }

    outColor = NeoWPF::UI::Color::FromArgb(it->second);
    return true;
}

const std::string& ThemeManager::GetActiveThemeName() const noexcept
{
    return m_activeThemeName;
}

const ThemeDefinition* ThemeManager::GetActiveTheme() const noexcept
{
    const auto it = m_themes.find(m_activeThemeName);
    return it != m_themes.end() ? &it->second : nullptr;
}

void ThemeManager::Clear()
{
    m_themes.clear();
    m_paletteColors.clear();
    m_activeThemeName.clear();
}

}
