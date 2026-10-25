/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>
#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Brush.h>
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
    static const Properties::DependencyProperty& ForegroundProperty();
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
        Sets the foreground brush used for the label and check mark.
    */
        void SetForeground(std::shared_ptr<Brush> brush);

    /*
        @summary
        Returns the foreground brush used for the label and check mark.
    */
        std::shared_ptr<Brush> GetForeground() const;

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
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Positions the checkbox in its final layout bounds.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

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
    Thickness m_padding = Thickness(8);
    int m_fontSize = 14;
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

class CheckboxTemplate : public Templates::ControlTemplate {
public:
    CheckboxTemplate();
};

}
