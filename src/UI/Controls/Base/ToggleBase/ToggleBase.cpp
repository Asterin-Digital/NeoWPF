#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

ToggleBase::ToggleBase()
    : Content::ContentControl()
{
    InitializeVisualStates();
}

void ToggleBase::OnInitialize()
{
    UIElement::OnInitialize();
    RefreshVisualState();
}

void ToggleBase::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();
}

std::string ToggleBase::ComputeVisualState() const
{
    if (!IsEnabled()) {
        return "Disabled";
    }
    if (m_isPressed) {
        return m_isChecked ? "CheckedPressed" : "Pressed";
    }
    if (m_isMouseOver) {
        return m_isChecked ? "CheckedMouseOver" : "MouseOver";
    }
    return m_isChecked ? "Checked" : "Unchecked";
}

void ToggleBase::RefreshVisualState()
{
    ApplyVisualState(ComputeVisualState(), false);
}

void ToggleBase::ApplyVisualState(const std::string& stateName, bool useTransitions)
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
    InvalidateVisual();
}

void ToggleBase::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

void ToggleBase::UpdateContentTextFromState()
{
    if (!m_contentText.empty()) {
        return;
    }
}

void ToggleBase::SetChecked(bool value) noexcept
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("IsChecked", *this)) {
            SetValue(*property, value);
            return;
        }
    }

    if (m_isChecked == value) {
        return;
    }

    m_isChecked = value;
    RefreshVisualState();
    if (m_isChecked) {
        RaiseChecked();
    }
    else {
        RaiseUnchecked();
    }
}

void ToggleBase::Toggle() noexcept
{
    SetChecked(!m_isChecked);
}

void ToggleBase::SetContentText(const std::wstring& text)
{
    if (!IsApplyingPropertyValue()) {
        if (const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("ContentText", *this)) {
            SetValue(*property, text);
            return;
        }
    }

    m_contentText = text;
    InvalidateVisual();
}

void ToggleBase::RaiseChecked()
{
    Checked.Invoke();
}

void ToggleBase::RaiseUnchecked()
{
    Unchecked.Invoke();
}

void ToggleBase::RaiseClick()
{
    Click.Invoke();
}

void ToggleBase::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    m_isMouseOver = true;
    RefreshVisualState();
}

void ToggleBase::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    m_isMouseOver = false;
    m_isPressed = false;
    RefreshVisualState();
}

void ToggleBase::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    m_isMouseOver = true;
    m_isPressed = IsEnabled() && (buttonState & 0x01) != 0;
    RefreshVisualState();
}

void ToggleBase::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    const bool shouldToggle = IsEnabled() && m_isPressed && ((buttonState & 0x01) == 0);
    m_isPressed = false;
    if (shouldToggle) {
        Toggle();
        RaiseClick();
    }
    RefreshVisualState();
}

void ToggleBase::OnGotFocus()
{
    RefreshVisualState();
}

void ToggleBase::OnLostFocus()
{
    RefreshVisualState();
}

}
