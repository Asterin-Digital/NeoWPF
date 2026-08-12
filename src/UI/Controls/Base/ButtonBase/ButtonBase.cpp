#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

// hai hai!! if you need a base for an UI control that has button behavior, please use this
// instead of reimplementing behavior and state from scratch

namespace NeoWPF::UI {

ButtonBase::ButtonBase()
    : Content::ContentControl()
{
    InitializeVisualStates();
}

void ButtonBase::OnInitialize()
{
    UIElement::OnInitialize();
    ApplyVisualState("Normal", false);
}

void ButtonBase::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();
}

void ButtonBase::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;

    if (auto templateManager = GetTemplateVisualStateManager()) {
        templateManager->GoToState(this, stateName, useTransitions);
    }
    else if (m_visualStateManager) {
        m_visualStateManager->GoToState(this, stateName, useTransitions);
    }

    OnVisualStateApplied(stateName);
}

void ButtonBase::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

void ButtonBase::RaiseClick()
{
    if (m_command) {
        auto commandTarget = m_commandTarget ? m_commandTarget : this;

        if (auto routedCommand = std::dynamic_pointer_cast<NeoWPF::Input::RoutedCommand>(m_command)) {
            if (routedCommand->CanExecute(m_commandParameter, commandTarget)) {
                routedCommand->Execute(m_commandParameter, commandTarget);
            }
        }
        else if (m_command->CanExecute(m_commandParameter)) {
            m_command->Execute(m_commandParameter);
        }
    }
    Click.Invoke();
}

void ButtonBase::SetCommand(std::shared_ptr<NeoWPF::Input::ICommand> command)
{
    m_command = std::move(command);
    if (!m_commandTarget) {
        m_commandTarget = this;
    }
}

void ButtonBase::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", true);
    }
}

void ButtonBase::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("Normal", true);
    }
}

void ButtonBase::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("Pressed", true);
    }
}

void ButtonBase::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", true);
        RaiseClick();
    }
    else {
        ApplyVisualState("Disabled", true);
    }
}

}
