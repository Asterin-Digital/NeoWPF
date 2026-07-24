#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <cstdint>
#include <utility>

namespace NeoWPF::Core::ApplicationThemes {

ThemeManager& ThemeManager::Instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::RegisterBuiltInThemes()
{
    ThemeDefinition light;
    light.Name = "Light";
    light.Resources["Window.BackgroundColor"] = uint32_t(0xFFF3F3F3u);
    light.Resources["Window.ForegroundColor"] = uint32_t(0xFF1E1E1Eu);
    light.Resources["Button.BackgroundColor"] = uint32_t(0xFFEEEEEEu);
    light.Resources["Button.BorderColor"] = uint32_t(0xFF707070u);
    light.Resources["Button.ForegroundColor"] = uint32_t(0xFF000000u);
    light.Resources["Checkbox.BorderColor"] = uint32_t(0xFF202020u);
    light.Resources["Checkbox.BoxColor"] = uint32_t(0xFFFFFFFFu);
    light.Resources["Checkbox.TextColor"] = uint32_t(0xFF000000u);
    light.Resources["TextBox.BackgroundColor"] = uint32_t(0xFFFFFFFFu);
    light.Resources["TextBox.BorderColor"] = uint32_t(0xFFABADB3u);
    light.Resources["TextBox.ForegroundColor"] = uint32_t(0xFF000000u);
    light.Resources["ScrollBar.TrackColor"] = uint32_t(0xFFEAEAEAu);
    light.Resources["ScrollBar.ThumbColor"] = uint32_t(0xFFCDCDCDu);

    ThemeDefinition dark;
    dark.Name = "Dark";
    dark.Resources["Window.BackgroundColor"] = uint32_t(0xFF1E1E1Eu);
    dark.Resources["Window.ForegroundColor"] = uint32_t(0xFFEDEDEDu);
    dark.Resources["Button.BackgroundColor"] = uint32_t(0xFF3A3A3Au);
    dark.Resources["Button.BorderColor"] = uint32_t(0xFF707070u);
    dark.Resources["Button.ForegroundColor"] = uint32_t(0xFFEDEDEDu);
    dark.Resources["Checkbox.BorderColor"] = uint32_t(0xFF9E9E9Eu);
    dark.Resources["Checkbox.BoxColor"] = uint32_t(0xFF2A2A2Au);
    dark.Resources["Checkbox.TextColor"] = uint32_t(0xFFEDEDEDu);
    dark.Resources["TextBox.BackgroundColor"] = uint32_t(0xFF2F2F2Fu);
    dark.Resources["TextBox.BorderColor"] = uint32_t(0xFF666666u);
    dark.Resources["TextBox.ForegroundColor"] = uint32_t(0xFFEDEDEDu);
    dark.Resources["ScrollBar.TrackColor"] = uint32_t(0xFFEAEAEAu);
    dark.Resources["ScrollBar.ThumbColor"] = uint32_t(0xFFCDCDCDu);

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
    m_activeThemeName.clear();
}

}
