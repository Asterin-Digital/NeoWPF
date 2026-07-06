#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

void Button::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();

    auto group = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    group->AddState(std::make_shared<Styling::VisualState>("Normal"));
    group->AddState(std::make_shared<Styling::VisualState>("MouseOver"));
    group->AddState(std::make_shared<Styling::VisualState>("Pressed"));
    group->AddState(std::make_shared<Styling::VisualState>("Disabled"));

    m_visualStateManager->AddGroup(group);
}

void Button::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (!m_visualStateManager) {
        InitializeVisualStates();
    }

    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;
    m_visualStateManager->GoToState(this, stateName, useTransitions);

    uint32_t resolvedColor = 0xFF3A3A3Au;
    std::string propertyName = "VisualState." + stateName + ".Background";
    if (!TryGetProperty(propertyName, resolvedColor)) {
        if (stateName == "Normal") {
            resolvedColor = 0xFF3A3A3Au;
        }
    }

    SetBackgroundColor(resolvedColor);
}

void Button::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", false);
    }
}

void Button::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (IsEnabled()) {
        ApplyVisualState("Normal", false);
    }
}

void Button::OnMouseDown(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("Pressed", false);
    }
}

void Button::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    if (IsEnabled()) {
        ApplyVisualState("MouseOver", false);
        RaiseClick();
    }
    else {
        ApplyVisualState("Disabled", false);
    }
}

} // namespace NeoWPF::UI
