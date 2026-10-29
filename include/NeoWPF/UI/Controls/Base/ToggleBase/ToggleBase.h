/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

/*
    @summary
    Provides shared checked-state and interaction behavior for toggle controls.
*/
class ToggleBase : public NeoWPF::UI::Content::ContentControl {
public:
    static const Properties::DependencyProperty& IsCheckedProperty();
    static const Properties::DependencyProperty& ContentTextProperty();

    ToggleBase();
    virtual ~ToggleBase() = default;

    /*
        @summary
        Reports whether the toggle participates in hit testing.
    */
    bool IsHitTestVisible() const noexcept override { return true; }
    /*
        @summary
        Reports whether the toggle can currently be interacted with.
    */
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    /*
        @summary
        Returns whether the toggle is checked.
    */
    bool IsChecked() const noexcept { return m_isChecked; }
    /*
        @summary
        Sets the checked state of the toggle.

        @param value
        True to check the toggle.
    */
    virtual void SetChecked(bool value) noexcept;
    /*
        @summary
        Switches the toggle to the opposite checked state.
    */
    void Toggle() noexcept;

    /*
        @summary
        Sets the text displayed by the toggle.

        @param text
        Text to display.
    */
    void SetContentText(const std::wstring& text);
    /*
        @summary
        Returns the text displayed by the toggle.
    */
    const std::wstring& GetContentText() const noexcept { return m_contentText; }

    /*
        @summary
        Handles keyboard focus entering the toggle.

        @internal
    */
    void OnGotFocus() override;
    /*
        @summary
        Handles keyboard focus leaving the toggle.

        @internal
    */
    void OnLostFocus() override;

    NeoWPF::Core::Event<> Checked;
    NeoWPF::Core::Event<> Unchecked;
    NeoWPF::Core::Event<> Click;

protected:
    /*
        @summary
        Handles routed input for toggle interaction.

        @param args
        The routed event arguments.

        @internal
    */
    void OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args) override;
    void OnInitialize() override;
    void InitializeVisualStates();
    /*
        @internal
        @summary
        Computes the visual state from the toggle's current interaction state.
    */
    std::string ComputeVisualState() const;
    /*
        @internal
        @summary
        Applies the visual state matching the toggle's current state.
    */
    void RefreshVisualState();
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);
    void RaiseChecked();
    void RaiseUnchecked();
    void RaiseClick();
    virtual void OnVisualStateApplied(const std::string& stateName);
    virtual void UpdateContentTextFromState();

protected:
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;
    bool m_isChecked = false;
    bool m_isPressed = false;
    bool m_isMouseOver = false;
    std::wstring m_contentText;
};

}
