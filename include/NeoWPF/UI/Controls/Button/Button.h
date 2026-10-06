/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Text/FontStyle.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI::Styling {
    class Style;  
}

namespace NeoWPF::UI {

class UIElement;
class ButtonStyle;  
void RegisterButtonStyles();

/*
    @summary
    A simple button control

    @details
    This is the basic clickable thing you use for actions. It can show text, change colors,
    and raise `Click` when the user presses it.
*/
class Button : public NeoWPF::UI::ButtonBase {
public:
    static const Properties::DependencyProperty& TextProperty();
    static const Properties::DependencyProperty& TextColorProperty();
    static const Properties::DependencyProperty& ForegroundProperty();
    static const Properties::DependencyProperty& FontSizeProperty();
    static const Properties::DependencyProperty& FontFamilyProperty();
    static const Properties::DependencyProperty& FontWeightProperty();
    static const Properties::DependencyProperty& FontStyleProperty();
    static const Properties::DependencyProperty& TextHorizontalAlignmentProperty();
    static const Properties::DependencyProperty& TextVerticalAlignmentProperty();

    Button();

    /*
        @summary
        Creates a new button with some starting text.

        @param text
        The text to show inside the button.
    */
    Button(const std::wstring& text);

    ~Button();

    /*
        @summary
        Replaces the button text.

        @param text
        The new text to display.
    */
    void SetText(const std::wstring& text);

    /*
        @summary
        Attaches a live localization expression to the button text property.

        @param key
        The application-owned localization key to resolve.

        @details
        The text updates when localization changes. Calling SetText replaces the locale value.
    */
    void SetLocale(const std::wstring& key);

    /*
        @summary
        Returns the current button text.
    */
    const std::wstring& GetText() const noexcept { return m_text; }

    /*
        @summary
        Sets the horizontal alignment used for the button text.
    */
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;

    /*
        @summary
        Sets the vertical alignment used for the button text.
    */
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;

    /*
        @summary
        Sets the text color.

        @param color
        ARGB color value.
    */
    void SetTextColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the current text color.
    */
    NeoWPF::UI::Color GetTextColor() const noexcept { return m_textColor; }

    /*
        @summary
        Sets the font size used for the button text.

        @param fontSize
        Size in pixels.
    */
    void SetFontSize(int fontSize) noexcept;

    /*
        @summary
        Returns the current font size.
    */
    int GetFontSize() const noexcept { return m_fontSize; }

    /*
    * @todo Verify if this is correct later
    */
    void SetFontFamily(const std::wstring& fontFamily) noexcept;

    /*
       @summary
       Returns the current font family.
    */
    const std::wstring& GetFontFamily() const noexcept { return m_fontFamily; }

    /*
        @summary
        Sets the font weight used to measure and render text.
    */
    void SetFontWeight(int fontWeight) noexcept;

    /*
        @summary
        Returns the current font weight used to measure and render text.
    */
    int GetFontWeight() const noexcept { return m_fontWeight; }

    /*
        @summary
        Sets the font style used to measure and render text.
    */
    void SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept;

    /*
        @summary
        Returns the current font style used to measure and render text.
    */
    NeoWPF::UI::FontStyle GetFontStyle() const noexcept { return m_fontStyle; }

    /*
        @summary
        Sets the padding around the text inside the button.

        @param padding
        Padding in pixels.
    */
    void SetPadding(Thickness padding) noexcept;

    /*
        @summary
        Returns the current padding.
    */
    Thickness GetPadding() const noexcept;

    /*
        @summary
        Sets the background color.

        @param color
        ARGB color value.
    */
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the current background color.
    */
    using Control::GetBackgroundColor;

    /*
        @summary
        Sets the border color.

        @param color
        ARGB color value.
    */
    void SetBorderColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the current border color.
    */
    using Control::GetBorderColor;

    /*
        @summary
        Measures the button and decides how much space it needs.

        @param availableWidth
        Width available for layout.

        @param availableHeight
        Height available for layout.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Positions the button inside its box

        @param finalWidth
        The width for the button.

        @param finalHeight
        The height for the button.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Builds the render commands for the button.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    void OnPaddingChanged(Thickness padding) override;
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderThicknessChanged(double thickness) override;

private:
    using ButtonBase::InitializeVisualStates;
    using ButtonBase::ApplyVisualState;
    using ButtonBase::RaiseClick;

private:
    std::wstring m_text;
    NeoWPF::UI::Color m_textColor;
    int m_fontSize = 16;
    std::wstring m_fontFamily;
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;

private:
    int m_padding = 8;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;
    std::size_t m_themeAppliedSubscriptionId = 0;
    int m_appliedThemeFontSize = 0;
    Thickness m_appliedThemePadding{};
    bool m_hasAppliedThemeFontSize = false;
    bool m_hasAppliedThemePadding = false;
};

class ButtonTemplate : public Templates::ControlTemplate {
public:
    ButtonTemplate();
};

}
