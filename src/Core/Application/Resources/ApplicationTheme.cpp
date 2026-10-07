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

    auto snapshot = std::make_shared<const ThemeDefinition>(theme);
    std::unique_lock lock(m_mutex);
    m_themes[theme.Name] = std::move(snapshot);
}

bool ThemeManager::HasTheme(const std::string& name) const
{
    std::shared_lock lock(m_mutex);
    return m_themes.find(name) != m_themes.end();
}

bool ThemeManager::ApplyTheme(const std::string& name)
{
    std::shared_ptr<const ThemeDefinition> theme;
    {
        std::shared_lock lock(m_mutex);
        const auto it = m_themes.find(name);
        if (it == m_themes.end()) {
            return false;
        }
        theme = it->second;
    }

    return ApplyTheme(*theme);
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
    std::unordered_map<std::string, std::any> resources;
    resources.reserve(theme.Palette.size() + theme.Resources.size());

    for (const auto& [paletteColor, value] : theme.Palette) {
        const auto* key = GetPaletteResourceKey(paletteColor);
        if (key) {
            resources.emplace(key, value);
        }
    }

    resources.insert(theme.Resources.begin(), theme.Resources.end());
    NeoWPF::Core::ApplicationResources::Resources::SetResources(resources);

    NeoWPF::Diagnostics::LogInfo(("Applied application theme: " + theme.Name).c_str());
}

bool ThemeManager::SetActiveTheme(const std::string& name)
{
    std::shared_ptr<const ThemeDefinition> themeSnapshot;
    {
        std::unique_lock lock(m_mutex);
        const auto it = m_themes.find(name);
        if (it == m_themes.end() || !it->second || it->second->Name.empty()) {
            return false;
        }

        themeSnapshot = it->second;
        m_activeTheme = themeSnapshot;
    }

    ApplyThemeResources(*themeSnapshot);
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
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->Palette.find(paletteColor);
    if (it == activeTheme->Palette.end()) {
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
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->FontSizes.find(fontSize);
    if (it == activeTheme->FontSizes.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetPadding(ThemePadding padding, NeoWPF::UI::Thickness& outValue) const
{
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->Paddings.find(padding);
    if (it == activeTheme->Paddings.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetBorderThickness(ThemeBorderThickness borderThickness, NeoWPF::UI::Thickness& outValue) const
{
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->BorderThicknesses.find(borderThickness);
    if (it == activeTheme->BorderThicknesses.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetMetric(ThemeMetric metric, double& outValue) const
{
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->Metrics.find(metric);
    if (it == activeTheme->Metrics.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

bool ThemeManager::TryGetCornerRadius(ThemeCornerRadius cornerRadius, NeoWPF::UI::CornerRadius& outValue) const
{
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }

    if (!activeTheme) {
        return false;
    }

    const auto it = activeTheme->CornerRadii.find(cornerRadius);
    if (it == activeTheme->CornerRadii.end()) {
        return false;
    }

    outValue = it->second;
    return true;
}

std::string ThemeManager::GetActiveThemeName() const
{
    std::shared_ptr<const ThemeDefinition> activeTheme;
    {
        std::shared_lock lock(m_mutex);
        activeTheme = m_activeTheme;
    }
    return activeTheme ? activeTheme->Name : std::string{};
}

std::shared_ptr<const ThemeDefinition> ThemeManager::GetActiveTheme() const
{
    std::shared_lock lock(m_mutex);
    return m_activeTheme;
}

void ThemeManager::Clear()
{
    std::unique_lock lock(m_mutex);
    m_themes.clear();
    m_activeTheme.reset();
}

}
