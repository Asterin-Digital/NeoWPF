/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Events/Notification.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <any>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace NeoWPF::Core::ApplicationThemes {

/*
    @summary
    Identifies a shared color role in an application theme.
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
    Focus,
    Track,
    Thumb,
};

/*
    @summary
    Identifies a shared font-size role in an application theme.
*/
enum class ThemeFontSize {
    Control,
};

/*
    @summary
    Identifies a control padding role in an application theme.
*/
enum class ThemePadding {
    Button,
    TextBlock,
    TextBox,
    Checkbox,
    Border,
    ComboBox,
    ComboBoxItem,
    ListBox,
    ListBoxItem,
    ProgressBar,
    RadioButton,
    ScrollBar,
    Slider,
};

/*
    @summary
    Identifies a shared control-size role in an application theme.
*/
enum class ThemeMetric {
    ScrollBarThumbSize,
    ProgressBarBorderThickness,
    SliderBorderThickness,
    SliderThumbLength,
    ScrollBarBorderThickness,
};

/*
    @summary
    Identifies a control border-thickness role in an application theme.
*/
enum class ThemeBorderThickness {
    Border,
};

/*
    @summary
    Identifies a control corner-radius role in an application theme.
*/
enum class ThemeCornerRadius {
    Control,
    Checkbox,
};

/*
    @summary
    Defines the resources and shared style values associated with a theme.
*/
struct ThemeDefinition {
    /*
        @summary
        Name used to register and select the theme.
    */
    std::string Name;

    /*
        @summary
        Resources applied when the theme is selected.
    */
    std::unordered_map<std::string, std::any> Resources;

    /*
        @summary
        Shared color values keyed by palette role.
    */
    std::unordered_map<PaletteColor, std::uint32_t> Palette;

    /*
        @summary
        Font sizes keyed by shared theme role.
    */
    std::unordered_map<ThemeFontSize, int> FontSizes;

    /*
        @summary
        Padding values keyed by control theme role.
    */
    std::unordered_map<ThemePadding, NeoWPF::UI::Thickness> Paddings;

    /*
        @summary
        Border thickness values keyed by control theme role.
    */
    std::unordered_map<ThemeBorderThickness, NeoWPF::UI::Thickness> BorderThicknesses;

    /*
        @summary
        Numeric control metrics keyed by theme role.
    */
    std::unordered_map<ThemeMetric, double> Metrics;

    /*
        @summary
        Corner radii keyed by control theme role.
    */
    std::unordered_map<ThemeCornerRadius, NeoWPF::UI::CornerRadius> CornerRadii;
};

/*
    @summary
    Registers, applies, and queries application themes.
*/
class ThemeManager {
public:
    /*
        @summary
        Returns the process-wide theme manager.

        @returns
        The shared theme manager instance.
    */
    static ThemeManager& Instance();

    /*
        @summary
        Registers the built-in Light Mint and Dark Mint themes.

        @internal
    */
    void RegisterBuiltInThemes();

    /*
        @summary
        Registers or replaces a named theme.

        @param theme
        Theme definition to register; definitions with an empty name are ignored.
    */
    void RegisterTheme(const ThemeDefinition& theme);

    /*
        @summary
        Checks whether a theme with the given name is registered.

        @param name
        Theme name to find.

        @returns
        True when the theme is registered.
    */
    bool HasTheme(const std::string& name) const;

    /*
        @summary
        Applies a registered theme's resources and style values.

        @param name
        Name of the registered theme to apply.

        @returns
        True when the theme exists and was applied.
    */
    bool ApplyTheme(const std::string& name);

    /*
        @summary
        Applies the resources and style values in a theme definition.

        @param theme
        Theme definition to apply; definitions with an empty name are rejected.

        @returns
        True when the definition was applied.

        @details
        Applying a definition does not change the active theme name.
    */
    bool ApplyTheme(const ThemeDefinition& theme);

    /*
        @summary
        Applies a registered theme and sets it as the active theme.

        @param name
        Name of the registered theme to activate.

        @returns
        True when the theme exists and became active.
    */
    bool SetActiveTheme(const std::string& name);

    /*
        @summary
        Subscribes to notifications sent after a theme is applied.

        @param handler
        Callback invoked after theme resources and palette values are updated.

        @returns
        A nonzero subscription identifier, or zero when the handler is empty.

        @details
        The callback runs synchronously on the thread that applies the theme.
    */
    std::size_t SubscribeThemeApplied(std::function<void()> handler);

    /*
        @summary
        Removes a theme-applied subscription.

        @param subscriptionId
        Identifier returned by SubscribeThemeApplied.
    */
    void UnsubscribeThemeApplied(std::size_t subscriptionId);

    /*
        @summary
        Retrieves the active color associated with a shared palette role.

        @param paletteColor
        Shared palette role to retrieve.

        @param outColor
        Receives the resolved color.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetPaletteColor(PaletteColor paletteColor, NeoWPF::UI::Color& outColor) const;

    /*
        @summary
        Retrieves the active font size associated with a shared theme role.

        @param fontSize
        Shared font-size role to retrieve.

        @param outValue
        Receives the resolved font size.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetFontSize(ThemeFontSize fontSize, int& outValue) const;

    /*
        @summary
        Retrieves the active Button padding associated with a theme role.

        @param padding
        Button padding role to retrieve.

        @param outValue
        Receives the resolved padding.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetPadding(ThemePadding padding, NeoWPF::UI::Thickness& outValue) const;

    /*
        @summary
        Retrieves the active border thickness associated with a theme role.

        @param borderThickness
        Border-thickness role to retrieve.

        @param outValue
        Receives the resolved border thickness.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetBorderThickness(ThemeBorderThickness borderThickness, NeoWPF::UI::Thickness& outValue) const;

    /*
        @summary
        Retrieves the active metric associated with a theme role.

        @param metric
        Metric role to retrieve.

        @param outValue
        Receives the resolved metric.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetMetric(ThemeMetric metric, double& outValue) const;

    /*
        @summary
        Retrieves the active corner radius associated with a theme role.

        @param cornerRadius
        Control corner-radius role to retrieve.

        @param outValue
        Receives the resolved corner radius.

        @returns
        True when the active theme defines the requested role.
    */
    bool TryGetCornerRadius(ThemeCornerRadius cornerRadius, NeoWPF::UI::CornerRadius& outValue) const;

    /*
        @summary
        Returns the name of the active theme.

        @returns
        An owned copy of the active theme name, or an empty string when no theme is active.
    */
    std::string GetActiveThemeName() const;

    /*
        @summary
        Returns a retained snapshot of the registered active theme.

        @returns
        A shared pointer to the active definition, or null when no registered theme is active.
    */
    std::shared_ptr<const ThemeDefinition> GetActiveTheme() const;

    /*
        @summary
        Removes registered themes and clears active theme values.
    */
    void Clear();

private:
    ThemeManager() = default;

    /*
        @summary
        Applies theme resources and style values without changing the active theme name.

        @param theme
        Theme definition to apply.

        @internal
    */
    void ApplyThemeResources(const ThemeDefinition& theme);

    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<const ThemeDefinition>> m_themes;
    std::shared_ptr<const ThemeDefinition> m_activeTheme;
    NeoWPF::Core::Notification<> m_themeApplied;
};

/*
    @summary
    Returns the active color for a shared palette role.

    @param paletteColor
    Palette role to retrieve.

    @returns
    The active color, or the default color when the role is not defined.
*/
NeoWPF::UI::Color GetPaletteColor(PaletteColor paletteColor);

}
