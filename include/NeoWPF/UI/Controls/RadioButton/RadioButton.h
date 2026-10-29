/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <NeoWPF/UI/Controls/Base/GroupedToggleBase/GroupedToggleBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/UI/Text/FontStyle.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <cstddef>

namespace NeoWPF::UI {

class RadioButtonTemplate;

/*
    @summary
    A mutually exclusive toggle control for selecting one option in a group.

    @details
    User activation selects a radio button without clearing an already selected
    button. Programmatic calls to SetChecked(false) may still clear it.
*/
class RadioButton : public GroupedToggleBase {
public:
    static const Properties::DependencyProperty& TextProperty();
    static const Properties::DependencyProperty& ForegroundProperty();
    static const Properties::DependencyProperty& PaddingProperty();
    static const Properties::DependencyProperty& FontSizeProperty();
    static const Properties::DependencyProperty& FontFamilyProperty();
    static const Properties::DependencyProperty& FontWeightProperty();
    static const Properties::DependencyProperty& FontStyleProperty();
    static const Properties::DependencyProperty& IsCheckedProperty();
    static const Properties::DependencyProperty& GroupNameProperty();

    RadioButton();
    ~RadioButton() override;

    /*
        @summary
        Sets the text displayed by the radio button.

        @param text
        Text to display.
    */
    void SetText(const std::wstring& text);
    /*
        @summary
        Returns the text displayed by the radio button.
    */
    const std::wstring& GetText() const noexcept { return m_text; }
    /*
        @summary
        Sets the localization key used for the radio button text.

        @param key
        Localization key.
    */
    void SetLocale(const std::wstring& key);
    /*
        @summary
        Sets whether the radio button is selected.

        @param value
        True to select the radio button.
    */
    void SetChecked(bool value) noexcept override;
    /*
        @summary
        Sets the horizontal alignment of the radio button text.
    */
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;
    /*
        @summary
        Returns the horizontal alignment of the radio button text.
    */
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;
    /*
        @summary
        Sets the vertical alignment of the radio button text.
    */
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;
    /*
        @summary
        Returns the vertical alignment of the radio button text.
    */
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;
    /*
        @summary
        Sets the brush used to draw the radio button text.
    */
    void SetForeground(std::shared_ptr<Brush> brush);
    /*
        @summary
        Returns the brush used to draw the radio button text.
    */
    std::shared_ptr<Brush> GetForeground() const;
    /*
        @summary
        Sets the space around the radio button content.
    */
    void SetPadding(Thickness padding) noexcept;
    /*
        @summary
        Returns the space around the radio button content.
    */
    Thickness GetPadding() const noexcept;
    /*
        @summary
        Sets the radio button text size.
    */
    void SetFontSize(int fontSize) noexcept;
    /*
        @summary
        Returns the radio button text size.
    */
    int GetFontSize() const noexcept;
    /*
        @summary
        Sets the radio button text font family.
    */
    void SetFontFamily(const std::wstring& fontFamily) noexcept;
    /*
        @summary
        Returns the radio button text font family.
    */
    const std::wstring& GetFontFamily() const noexcept { return m_fontFamily; }
    /*
        @summary
        Sets the radio button text font weight.
    */
    void SetFontWeight(int fontWeight) noexcept;
    /*
        @summary
        Returns the radio button text font weight.
    */
    int GetFontWeight() const noexcept;
    /*
        @summary
        Sets the radio button text font style.
    */
    void SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept;
    /*
        @summary
        Returns the radio button text font style.
    */
    NeoWPF::UI::FontStyle GetFontStyle() const noexcept;

    /*
        @summary
        Measures the radio button within the available layout space.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;
    /*
        @summary
        Arranges the radio button within its final layout size.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;
    /*
        @summary
        Adds the radio button drawing operations to a command list.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    /*
        @summary
        Handles routed input for radio button interaction.

        @param args
        The routed event arguments.

        @internal
    */
    void OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args) override;

    /*
        @summary
        Handles a visual state after it has been applied.

        @param stateName
        Name of the applied state.

        @internal
    */
    void OnVisualStateApplied(const std::string& stateName) override;
    /*
        @summary
        Determines whether a visual is a grouped radio button peer.

        @param visual
        Visual to check.

        @internal
    */
    bool IsGroupedPeer(const Visual* visual) const noexcept override;

private:
    std::wstring m_text;
    std::wstring m_fontFamily;
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;
    std::size_t m_themeAppliedSubscriptionId = 0;
};
/*
    @summary
    Defines the control template for a radio button.
*/
class RadioButtonTemplate : public Templates::ControlTemplate {
public:
    RadioButtonTemplate();
};

}