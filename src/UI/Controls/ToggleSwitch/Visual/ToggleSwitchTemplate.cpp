#include <NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

namespace {

std::shared_ptr<Brush> GetTrackBrush(const std::string& stateName)
{
    using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;

    if (stateName == "Disabled") {
        return GetPaletteBrush(PaletteBrush::DisabledSurface);
    }
    if (stateName == "Pressed") {
        return GetPaletteBrush(PaletteBrush::AccentPressed);
    }
    if (stateName == "Checked" || stateName == "CheckedMouseOver" ||
        stateName == "CheckedPressed") {
        return GetPaletteBrush(PaletteBrush::Accent);
    }
    if (stateName == "MouseOver") {
        return GetPaletteBrush(PaletteBrush::AccentHover);
    }
    return GetPaletteBrush(PaletteBrush::Track);
}

std::shared_ptr<Brush> GetThumbBrush(const std::string& stateName)
{
    using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;

    if (stateName == "Disabled") {
        return GetPaletteBrush(PaletteBrush::Track);
    }
    if (stateName == "Checked" || stateName == "CheckedMouseOver" ||
        stateName == "CheckedPressed") {
        return GetPaletteBrush(PaletteBrush::Surface);
    }
    return GetPaletteBrush(PaletteBrush::Thumb);
}

void ApplyTrackState(UIElement*, Visual* visual, const std::string& stateName)
{
    if (auto* track = dynamic_cast<Templates::TemplateVisual*>(visual)) {
        track->SetFillBrush(GetTrackBrush(stateName));
    }
}

void ApplyThumbState(UIElement*, Visual* visual, const std::string& stateName)
{
    if (auto* thumb = dynamic_cast<Templates::TemplateVisual*>(visual)) {
        thumb->SetFillBrush(GetThumbBrush(stateName));
    }
}

}

ToggleSwitchTemplate::ToggleSwitchTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Track));
    track->SetBorderThickness(0.0);

    auto thumb = std::make_shared<Templates::TemplateVisual>("PART_Thumb");
    thumb->SetFillBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Thumb));
    thumb->SetBorderThickness(0.0);

    root->AddVisualChild(track);
    root->AddVisualChild(thumb);
    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Thumb", thumb);

    auto manager = std::make_shared<Styling::VisualStateManager>();
    auto states = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    for (const char* name : {"Unchecked", "Checked", "MouseOver", "Pressed",
                             "CheckedMouseOver", "CheckedPressed", "Disabled"}) {
        const std::string stateName(name);
        auto state = std::make_shared<Styling::VisualState>(stateName);
        state->AddSetter("PART_Track", [stateName](UIElement* element, Visual* visual) {
            ApplyTrackState(element, visual, stateName);
        });
        state->AddSetter("PART_Thumb", [stateName](UIElement* element, Visual* visual) {
            ApplyThumbState(element, visual, stateName);
        });
        states->AddState(state);
    }
    manager->AddGroup(states);
    SetVisualStateManager(manager);
}

}