#pragma once

#include <memory>
#include <string>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
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

class CheckboxTemplate;

/*
    @summary
    A toggle control for on/off selection.
*/
class Checkbox : public NeoWPF::UI::ToggleBase {
public:
    static const Properties::DependencyProperty& TextProperty();
    static const Properties::DependencyProperty& TextColorProperty();
    static const Properties::DependencyProperty& BackgroundColorProperty();
    static const Properties::DependencyProperty& BorderColorProperty();
    static const Properties::DependencyProperty& PaddingProperty();
    static const Properties::DependencyProperty& FontSizeProperty();
    static const Properties::DependencyProperty& FontWeightProperty();
    static const Properties::DependencyProperty& FontStyleProperty();
    static const Properties::DependencyProperty& IsCheckedProperty();

    Checkbox();

    ~Checkbox();

    /*
        @summary
        Sets the checkbox label text.

        @param text
        The text to show next to the checkbox.
    */
    void SetText(const std::wstring& text);

    /*
        @summary
        Attaches a live localization expression to the checkbox label.

        @param key
        The application-owned localization key to resolve.

        @details
        The text updates when localization changes. Calling SetText replaces the locale value.
    */
    void SetLocale(const std::wstring& key);

    /*
        @summary
        Sets the checked state of the checkbox.

        @param value
        True when checked, false when unchecked.
    */
    void SetChecked(bool value) noexcept override;

    /*
        @summary
        Returns the checkbox label text.
    */
    const std::wstring& GetText() const noexcept { return m_text; }

    /*
        @summary
        Sets the horizontal alignment used for the checkbox label.
    */
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;

    /*
        @summary
        Sets the vertical alignment used for the checkbox label.
    */
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;

    /*
        @summary
        Sets the text color.
    */
    void SetTextColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the text color.
    */
    NeoWPF::UI::Color GetTextColor() const noexcept;

    /*
        @summary
        Sets the background color.
    */
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the background color.
    */
    NeoWPF::UI::Color GetBackgroundColor() const noexcept;

    /*
        @summary
        Sets the border color.
    */
    void SetBorderColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Returns the border color.
    */
    NeoWPF::UI::Color GetBorderColor() const noexcept;

    /*
        @summary
        Sets the padding around the checkbox content.
    */
    void SetPadding(Thickness padding) noexcept;

    /*
        @summary
        Returns the current padding.
    */
    Thickness GetPadding() const noexcept;

    /*
        @summary
        Sets the font size of the checkbox label.
    */
    void SetFontSize(int fontSize) noexcept;

    /*
        @summary
        Returns the current font size of the checkbox label.
    */
    int GetFontSize() const noexcept;

    /*
        @summary
        Sets the font weight used to measure and render text.
    */
    void SetFontWeight(int fontWeight) noexcept;

    /*
        @summary
        Returns the current font weight used to measure and render text.
    */
    int GetFontWeight() const noexcept;

    /*
        @summary
        Sets the font style used to measure and render text.
    */
    void SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept;

    /*
        @summary
        Returns the current font style used to measure and render text.
    */
    NeoWPF::UI::FontStyle GetFontStyle() const noexcept;

    /*
        @summary
        Measures the checkbox for layout.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Positions the checkbox in its final layout bounds.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Builds the render commands for the checkbox.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    /*
        @summary
        Applies the named visual state.

        @internal
    */
    void OnVisualStateApplied(const std::string& stateName) override;

private:
    std::wstring m_text;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFF000000u);
    NeoWPF::UI::Color m_bgColor = NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu);
    NeoWPF::UI::Color m_borderColor = NeoWPF::UI::Color::FromArgb(0xFF202020u);
    Thickness m_padding = Thickness(8);
    int m_fontSize = 14;
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;
};

class CheckboxTemplate : public Templates::ControlTemplate {
public:
    CheckboxTemplate();
};

}
