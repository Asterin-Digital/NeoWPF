#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

void TextBox::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();

    auto group = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    group->AddState(std::make_shared<Styling::VisualState>("Normal"));
    group->AddState(std::make_shared<Styling::VisualState>("Focused"));
    group->AddState(std::make_shared<Styling::VisualState>("Disabled"));

    m_visualStateManager->AddGroup(group);
}

void TextBox::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (!m_visualStateManager) {
        InitializeVisualStates();
    }

    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;
    m_visualStateManager->GoToState(this, stateName, useTransitions);

    uint32_t resolvedBgColor = 0xFF2F2F2Fu;
    uint32_t resolvedBorderColor = 0xFF666666u;
    
    std::string bgPropertyName = "VisualState." + stateName + ".Background";
    std::string borderPropertyName = "VisualState." + stateName + ".Border";
    
    if (!TryGetProperty(bgPropertyName, resolvedBgColor)) {
        if (stateName == "Normal") {
            resolvedBgColor = 0xFF2F2F2Fu;
            resolvedBorderColor = 0xFF666666u;
        }
    }
    if (!TryGetProperty(borderPropertyName, resolvedBorderColor)) {
        if (stateName == "Normal") {
            resolvedBorderColor = 0xFF666666u;
        }
    }

    SetBackgroundColor(resolvedBgColor);
    SetBorderColor(resolvedBorderColor);
}

void TextBox::OnGotFocus()
{
    m_isFocused = true;
    InvalidateVisual();
    ApplyVisualState("Focused", false);
}

void TextBox::OnLostFocus()
{
    m_isFocused = false;
    InvalidateVisual();
    ApplyVisualState("Normal", false);
}

} // namespace NeoWPF::UI
