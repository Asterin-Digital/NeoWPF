/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <cstddef>
#include <memory>
#include <string>

namespace NeoWPF::UI {

class TextBlock;

/*
    @summary
    A labeled switch control with checked and unchecked states.
*/
class ToggleSwitch : public ToggleBase {
public:
    static const Properties::DependencyProperty& IsCheckedProperty();
    static const Properties::DependencyProperty& ContentTextProperty();

    ToggleSwitch();

    ~ToggleSwitch() override;

    /*
        @summary
        Sets the switch's checked state.

        @param value
        True when the switch is checked.
    */
    void SetChecked(bool value) noexcept override;

    /*
        @summary
        Sets the label displayed beside the switch.

        @param text
        The label text.
    */
    void SetContentText(const std::wstring& text);

    /*
        @summary
        Measures the switch and its optional label.

        @param availableWidth
        The available width.

        @param availableHeight
        The available height.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the switch and its optional label.

        @param finalWidth
        The assigned width.

        @param finalHeight
        The assigned height.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Builds the switch and label render commands.

        @param commands
        The command list to populate.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private: 
    void ApplyTheme();

    std::shared_ptr<TextBlock> m_contentTextElement;
    std::size_t m_themeAppliedSubscriptionId = 0;
};

class ToggleSwitchTemplate : public Templates::ControlTemplate {
public:
    ToggleSwitchTemplate();
};

}