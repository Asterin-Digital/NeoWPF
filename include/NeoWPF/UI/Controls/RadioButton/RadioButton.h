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

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const noexcept { return m_text; }
    void SetLocale(const std::wstring& key);
    void SetChecked(bool value) noexcept override;
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;
    void SetForeground(std::shared_ptr<Brush> brush);
    std::shared_ptr<Brush> GetForeground() const;
    void SetPadding(Thickness padding) noexcept;
    Thickness GetPadding() const noexcept;
    void SetFontSize(int fontSize) noexcept;
    int GetFontSize() const noexcept;
    void SetFontFamily(const std::wstring& fontFamily) noexcept;
    const std::wstring& GetFontFamily() const noexcept { return m_fontFamily; }
    void SetFontWeight(int fontWeight) noexcept;
    int GetFontWeight() const noexcept;
    void SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept;
    NeoWPF::UI::FontStyle GetFontStyle() const noexcept;

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
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

    void OnVisualStateApplied(const std::string& stateName) override;
    bool IsGroupedPeer(const Visual* visual) const noexcept override;

private:
    std::wstring m_text;
    std::wstring m_fontFamily;
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    std::shared_ptr<NeoWPF::UI::TextBlock> m_textContent;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

class RadioButtonTemplate : public Templates::ControlTemplate {
public:
    RadioButtonTemplate();
};

}