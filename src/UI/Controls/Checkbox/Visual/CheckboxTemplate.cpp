#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <memory>
#include <string>

namespace NeoWPF::UI {

namespace {
using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;

void ApplyBoxState(UIElement* element, Visual* visual, const std::string& stateName)
{
    auto* checkbox = dynamic_cast<Checkbox*>(element);
    auto* box = dynamic_cast<Templates::TemplateVisual*>(visual);
    if (!checkbox || !box) {
        return;
    }

    auto fill = checkbox->GetBackgroundBrush();
    auto border = checkbox->GetBorderBrush();
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    if (stateName == "Checked") {
        fill = GetPaletteBrush(PaletteBrush::Accent);
        border = GetPaletteBrush(PaletteBrush::Accent);
    } else if (stateName == "CheckedMouseOver") {
        fill = GetPaletteBrush(PaletteBrush::AccentHover);
        border = GetPaletteBrush(PaletteBrush::Focus);
    } else if (stateName == "CheckedPressed") {
        fill = GetPaletteBrush(PaletteBrush::AccentPressed);
        border = GetPaletteBrush(PaletteBrush::Accent);
    } else if (stateName == "Focused" || stateName == "MouseOver") {
        fill = GetPaletteBrush(PaletteBrush::AccentHover);
        border = GetPaletteBrush(PaletteBrush::Focus);
    } else if (stateName == "Pressed") {
        fill = GetPaletteBrush(PaletteBrush::AccentPressed);
        border = GetPaletteBrush(PaletteBrush::Accent);
    } else if (stateName == "Disabled") {
        fill = GetPaletteBrush(PaletteBrush::DisabledSurface);
        border = GetPaletteBrush(PaletteBrush::Border);
    }

    box->SetFillBrush(fill);
    box->SetBorderBrush(border);
    box->SetCornerRadius(checkbox->GetCornerRadius());
    checkbox->InvalidateVisual();
}

}

CheckboxTemplate::CheckboxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto box = std::make_shared<Templates::TemplateVisual>("PART_Box");
    box->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface));
    box->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    box->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Box", box);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Normal"); });
    commonStates->AddState(normalState);

    auto checkedState = std::make_shared<Styling::VisualState>("Checked");
    checkedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Checked"); });
    commonStates->AddState(checkedState);

    auto uncheckedState = std::make_shared<Styling::VisualState>("Unchecked");
    uncheckedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Unchecked"); });
    commonStates->AddState(uncheckedState);

    auto focusedState = std::make_shared<Styling::VisualState>("Focused");
    focusedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Focused"); });
    commonStates->AddState(focusedState);

    auto mouseOverState = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOverState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "MouseOver"); });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Pressed"); });
    commonStates->AddState(pressedState);

    auto checkedMouseOverState = std::make_shared<Styling::VisualState>("CheckedMouseOver");
    checkedMouseOverState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "CheckedMouseOver"); });
    commonStates->AddState(checkedMouseOverState);

    auto checkedPressedState = std::make_shared<Styling::VisualState>("CheckedPressed");
    checkedPressedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "CheckedPressed"); });
    commonStates->AddState(checkedPressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) { ApplyBoxState(element, visual, "Disabled"); });
    commonStates->AddState(disabledState);

    visualStateManager->AddGroup(commonStates);
    SetVisualStateManager(visualStateManager);
}

}
