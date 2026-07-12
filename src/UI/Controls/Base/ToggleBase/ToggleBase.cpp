#include <NeoWPF/UI/Controls/Base/ToggleBase/ToggleBase.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

// please dont reimplement this anywhere else for controls that need
// button and toggle like behavior!! id like to keep it organized and tidy :>

ToggleBase::ToggleBase()
    : Content::ContentControl()
{
    InitializeVisualStates();
    ApplyVisualState("Normal", false);
}

void ToggleBase::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();
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
    if (m_isChecked == value) {
        return;
    }

    m_isChecked = value;
    if (m_isChecked) {
        ApplyVisualState("Checked", false);
        RaiseChecked();
    }
    else {
        ApplyVisualState("Unchecked", false);
        RaiseUnchecked();
    }

    InvalidateVisual();
}

void ToggleBase::Toggle() noexcept
{
    SetChecked(!m_isChecked);
}

void ToggleBase::SetContentText(const std::wstring& text)
{
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
    if (IsEnabled()) {
        ApplyVisualState(m_isChecked ? "CheckedMouseOver" : "MouseOver", false);
    }
}

void ToggleBase::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState(m_isChecked ? "Checked" : "Normal", false);
    }
}

void ToggleBase::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState(m_isChecked ? "CheckedPressed" : "Pressed", false);
    }
}

void ToggleBase::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        Toggle();
        RaiseClick();
        ApplyVisualState(m_isChecked ? "CheckedMouseOver" : "MouseOver", false);
    }
    else {
        ApplyVisualState("Disabled", false);
    }
}

}
