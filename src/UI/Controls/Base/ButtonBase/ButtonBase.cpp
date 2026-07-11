#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

// hai hai!! if you need a base for an UI control that has button behavior, please use this
// instead of reimplementing behavior and state from scratch

namespace NeoWPF::UI {

ButtonBase::ButtonBase()
    : Content::ContentControl()
{
    InitializeVisualStates();
    ApplyVisualState("Normal", false);
}

void ButtonBase::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();

    auto group = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    group->AddState(std::make_shared<Styling::VisualState>("Normal"));
    group->AddState(std::make_shared<Styling::VisualState>("MouseOver"));
    group->AddState(std::make_shared<Styling::VisualState>("Pressed"));
    group->AddState(std::make_shared<Styling::VisualState>("Disabled"));

    m_visualStateManager->AddGroup(group);
}

void ButtonBase::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (!m_visualStateManager) {
        InitializeVisualStates();
    }

    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;
    m_visualStateManager->GoToState(this, stateName, useTransitions);
    OnVisualStateApplied(stateName);
}

void ButtonBase::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

void ButtonBase::RaiseClick()
{
    Click.Invoke();
}

void ButtonBase::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", false);
    }
}

void ButtonBase::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("Normal", false);
    }
}

void ButtonBase::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("Pressed", false);
    }
}

void ButtonBase::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", false);
        RaiseClick();
    }
    else {
        ApplyVisualState("Disabled", false);
    }
}

}
