#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>

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
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Command", *this)) {
            SetValue(*property, command);
            return;
        }
    }

    m_command = std::move(command);
    if (!m_commandTarget) {
        m_commandTarget = this;
    }
}

void ButtonBase::SetCommandParameter(const std::any& parameter)
{
    if (IsApplyingPropertyValue()) {
        m_commandParameter = parameter;
        return;
    }

    m_commandParameter = parameter;
    if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CommandParameter", *this)) {
        SetValueInternal(*property, parameter, Properties::PropertyValueSource::Local);
        return;
    }
}

void ButtonBase::SetCommandTarget(NeoWPF::UI::UIElement* target) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CommandTarget", *this)) {
            SetValue(*property, target);
            return;
        }
    }

    m_commandTarget = target;
}

void ButtonBase::OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args) {
        return;
    }

    const auto* routedEvent = args->GetRoutedEvent();
    if (routedEvent == &NeoWPF::Input::Core::PointerEnteredEvent()) {
        m_isMouseOver = true;
        if (IsEnabled()) {
            ApplyVisualState("MouseOver", true);
        }
        return;
    }

    if (routedEvent == &NeoWPF::Input::Core::PointerExitedEvent()) {
        m_isMouseOver = false;
        m_isPressed = false;
        if (IsEnabled()) {
            ApplyVisualState("Normal", true);
        }
        return;
    }

    auto* pointerArgs = dynamic_cast<NeoWPF::Input::Core::RoutedPointerEventArgs*>(args);
    if (!pointerArgs) {
        return;
    }

    if (routedEvent == &NeoWPF::Input::Core::PointerPressedEvent()) {
        if (IsEnabled() && pointerArgs->GetChangedButton() == 0x01u) {
            m_isPressed = true;
            ApplyVisualState("Pressed", true);
        }
        return;
    }

    if (routedEvent == &NeoWPF::Input::Core::PointerReleasedEvent()) {
        if (IsEnabled()) {
            const bool shouldClick = m_isPressed && pointerArgs->GetChangedButton() == 0x01u;
            m_isPressed = false;
            ApplyVisualState(m_isMouseOver ? "MouseOver" : "Normal", true);
            if (shouldClick) {
                RaiseClick();
            }
        } else {
            m_isPressed = false;
            ApplyVisualState("Disabled", true);
        }
        return;
    }

    if (routedEvent == &NeoWPF::Input::Core::PointerCanceledEvent()) {
        m_isPressed = false;
        ApplyVisualState(m_isMouseOver ? "MouseOver" : "Normal", true);
    }
}

}
