/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <any>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Input/Command.h>

namespace NeoWPF::UI {

/*
    @summary
    Provides shared command and interaction behavior for button controls.
*/
class ButtonBase : public NeoWPF::UI::Content::ContentControl, public NeoWPF::Input::ICommandSource {
public:
    static const Properties::DependencyProperty& CommandProperty();
    static const Properties::DependencyProperty& CommandParameterProperty();
    static const Properties::DependencyProperty& CommandTargetProperty();

    ButtonBase();
    virtual ~ButtonBase() = default;

    /*
        @summary
        Reports whether the button participates in hit testing.
    */
    bool IsHitTestVisible() const noexcept override { return true; }
    /*
        @summary
        Reports whether the button can currently be interacted with.
    */
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    /*
        @summary
        Returns the command associated with the button.
    */
    std::shared_ptr<NeoWPF::Input::ICommand> GetCommand() const noexcept override { return m_command; }
    /*
        @summary
        Assigns the command associated with the button.

        @param command
        Command to assign.
    */
    void SetCommand(std::shared_ptr<NeoWPF::Input::ICommand> command) override;

    /*
        @summary
        Returns the parameter passed to the button command.
    */
    std::any GetCommandParameter() const noexcept override { return m_commandParameter; }
    /*
        @summary
        Sets the parameter passed to the button command.

        @param parameter
        Command parameter.
    */
    void SetCommandParameter(const std::any& parameter) override;

    /*
        @summary
        Returns the element targeted by the button command.
    */
    NeoWPF::UI::UIElement* GetCommandTarget() const noexcept override { return m_commandTarget; }
    /*
        @summary
        Sets the element targeted by the button command.

        @param target
        Target element.
    */
    void SetCommandTarget(NeoWPF::UI::UIElement* target) noexcept override;

    NeoWPF::Core::Event<> Click;

protected:
    /*
        @summary
        Handles routed input for button interaction.

        @param args
        The routed event arguments.

        @internal
    */
    void OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args) override;
    void OnInitialize() override;
    void InitializeVisualStates();
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);
    void RaiseClick();
    virtual void OnVisualStateApplied(const std::string& stateName);

protected:
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;
    bool m_isPressed = false;
    bool m_isMouseOver = false;
    std::shared_ptr<NeoWPF::Input::ICommand> m_command;
    std::any m_commandParameter;
    NeoWPF::UI::UIElement* m_commandTarget = nullptr;
};

}
