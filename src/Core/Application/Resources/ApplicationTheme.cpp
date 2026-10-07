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
    case PaletteColor::Track: return "Palette.Track";
    case PaletteColor::Thumb: return "Palette.Thumb";
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
    light.Palette[PaletteColor::Track] = 0xFFEAEAEAu;
    light.Palette[PaletteColor::Thumb] = 0xFFCDCDCDu;
    //////////////////////////////////////////////////////////
    light.FontSizes[ThemeFontSize::Control] = 14;
    light.Paddings[ThemePadding::Button] = NeoWPF::UI::Thickness(8);
    light.Paddings[ThemePadding::TextBlock] = NeoWPF::UI::Thickness{};
    light.Paddings[ThemePadding::TextBox] = NeoWPF::UI::Thickness(6, 4, 6, 4);
    light.Paddings[ThemePadding::Checkbox] = NeoWPF::UI::Thickness(8);
    light.Paddings[ThemePadding::Border] = NeoWPF::UI::Thickness{};
    light.Paddings[ThemePadding::ComboBox] = NeoWPF::UI::Thickness(8);
    light.Paddings[ThemePadding::ComboBoxItem] = NeoWPF::UI::Thickness(8);
    light.Paddings[ThemePadding::ListBox] = NeoWPF::UI::Thickness(4);
    light.Paddings[ThemePadding::ListBoxItem] = NeoWPF::UI::Thickness(4);
    light.Paddings[ThemePadding::ProgressBar] = NeoWPF::UI::Thickness{};
    light.Paddings[ThemePadding::RadioButton] = NeoWPF::UI::Thickness(8);
    light.Paddings[ThemePadding::ScrollBar] = NeoWPF::UI::Thickness{};
    light.Paddings[ThemePadding::Slider] = NeoWPF::UI::Thickness{};
    light.BorderThicknesses[ThemeBorderThickness::Border] = NeoWPF::UI::Thickness(1);
    light.Metrics[ThemeMetric::ScrollBarThumbSize] = 24.0;
    light.Metrics[ThemeMetric::ProgressBarBorderThickness] = 0.0;
    light.Metrics[ThemeMetric::SliderBorderThickness] = 0.0;
    light.Metrics[ThemeMetric::SliderThumbLength] = 16.0;
    light.Metrics[ThemeMetric::ScrollBarBorderThickness] = 0.0;
    light.CornerRadii[ThemeCornerRadius::Control] = NeoWPF::UI::CornerRadius(4);
    light.CornerRadii[ThemeCornerRadius::Checkbox] = NeoWPF::UI::CornerRadius(2);

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
    dark.Palette[PaletteColor::Track] = 0xFF242624u;
    dark.Palette[PaletteColor::Thumb] = 0xFF5A625Bu;
    //////////////////////////////////////////////////////////
    dark.FontSizes[ThemeFontSize::Control] = 14;
    dark.Paddings[ThemePadding::Button] = NeoWPF::UI::Thickness(8);
    dark.Paddings[ThemePadding::TextBlock] = NeoWPF::UI::Thickness{};
    dark.Paddings[ThemePadding::TextBox] = NeoWPF::UI::Thickness(6, 4, 6, 4);
    dark.Paddings[ThemePadding::Checkbox] = NeoWPF::UI::Thickness(8);
    dark.Paddings[ThemePadding::Border] = NeoWPF::UI::Thickness{};
    dark.Paddings[ThemePadding::ComboBox] = NeoWPF::UI::Thickness(8);
    dark.Paddings[ThemePadding::ComboBoxItem] = NeoWPF::UI::Thickness(8);
    dark.Paddings[ThemePadding::ListBox] = NeoWPF::UI::Thickness(4);
    dark.Paddings[ThemePadding::ListBoxItem] = NeoWPF::UI::Thickness(4);
    dark.Paddings[ThemePadding::ProgressBar] = NeoWPF::UI::Thickness{};
    dark.Paddings[ThemePadding::RadioButton] = NeoWPF::UI::Thickness(8);
    dark.Paddings[ThemePadding::ScrollBar] = NeoWPF::UI::Thickness{};
    dark.Paddings[ThemePadding::Slider] = NeoWPF::UI::Thickness{};
    dark.BorderThicknesses[ThemeBorderThickness::Border] = NeoWPF::UI::Thickness(1);
    dark.Metrics[ThemeMetric::ScrollBarThumbSize] = 24.0;
    dark.Metrics[ThemeMetric::ProgressBarBorderThickness] = 0.0;
    dark.Metrics[ThemeMetric::SliderBorderThickness] = 0.0;
    dark.Metrics[ThemeMetric::SliderThumbLength] = 16.0;
    dark.Metrics[ThemeMetric::ScrollBarBorderThickness] = 0.0;
    dark.CornerRadii[ThemeCornerRadius::Control] = NeoWPF::UI::CornerRadius(4);
    dark.CornerRadii[ThemeCornerRadius::Checkbox] = NeoWPF::UI::CornerRadius(2);

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

    ApplyThemeResources(theme);
    m_themeApplied.Publish();
    return true;
}

void ThemeManager::ApplyThemeResources(const ThemeDefinition& theme)
{
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

    for (const auto& [fontSize, value] : theme.FontSizes) {
        m_fontSizes[fontSize] = value;
    }

    for (const auto& [padding, value] : theme.Paddings) {
        m_paddings[padding] = value;
    }

    for (const auto& [borderThickness, value] : theme.BorderThicknesses) {
        m_borderThicknesses[borderThickness] = value;
    }

    for (const auto& [metric, value] : theme.Metrics) {
        m_metrics[metric] = value;
    }

    for (const auto& [cornerRadius, value] : theme.CornerRadii) {
        m_cornerRadii[cornerRadius] = value;
    }

    NeoWPF::Diagnostics::LogInfo(("Applied application theme: " + theme.Name).c_str());
}

bool ThemeManager::SetActiveTheme(const std::string& name)
{
    const auto it = m_themes.find(name);
    if (it == m_themes.end() || it->second.Name.empty()) {
        return false;
    }

    m_activeThemeName = name;
    ApplyThemeResources(it->second);
    m_themeApplied.Publish();
    return true;
}

std::size_t ThemeManager::SubscribeThemeApplied(std::function<void()> handler)
{
    return m_themeApplied.Subscribe(std::move(handler));
}

void ThemeManager::UnsubscribeThemeApplied(std::size_t subscriptionId)
{
    m_themeApplied.Unsubscribe(subscriptionId);
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

NeoWPF::UI::Color GetPaletteColor(PaletteColor paletteColor)
{
    NeoWPF::UI::Color color;
    ThemeManager::Instance().TryGetPaletteColor(paletteColor, color);
    return color;
}

bool ThemeManager::TryGetFontSize(ThemeFontSize fontSize, int& outValue) const
{
    const auto it = m_fontSizes.find(fontSize);
    if (it == m_fontSizes.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetPadding(ThemePadding padding, NeoWPF::UI::Thickness& outValue) const
{
    const auto it = m_paddings.find(padding);
    if (it == m_paddings.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetBorderThickness(ThemeBorderThickness borderThickness, NeoWPF::UI::Thickness& outValue) const
{
    const auto it = m_borderThicknesses.find(borderThickness);
    if (it == m_borderThicknesses.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetMetric(ThemeMetric metric, double& outValue) const
{
    const auto it = m_metrics.find(metric);
    if (it == m_metrics.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetCornerRadius(ThemeCornerRadius cornerRadius, NeoWPF::UI::CornerRadius& outValue) const
{
    const auto it = m_cornerRadii.find(cornerRadius);
    if (it == m_cornerRadii.end()) {
        return false;
    }

    outValue = it->second;
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
    m_fontSizes.clear();
    m_paddings.clear();
    m_borderThicknesses.clear();
    m_metrics.clear();
    m_cornerRadii.clear();
    m_activeThemeName.clear();
}

}
