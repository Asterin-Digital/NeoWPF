#include <NeoWPF/UI/Controls/RadioButton/RadioButton.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

RadioButtonTemplate::RadioButtonTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);
    auto circle = std::make_shared<Templates::TemplateVisual>("PART_Circle");
    circle->SetFillColor(0xFFFFFFFFu);
    circle->SetBorderColor(0xFF202020u);
    circle->SetBorderThickness(1);
    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    AddTemplateChild("PART_Circle", circle);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto manager = std::make_shared<Styling::VisualStateManager>();
    auto states = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    for (const char* name : {"Normal", "Unchecked", "Checked", "MouseOver", "Pressed", "CheckedMouseOver", "CheckedPressed", "Focused", "Disabled"}) {
        states->AddState(std::make_shared<Styling::VisualState>(name));
    }
    manager->AddGroup(states);
    SetVisualStateManager(manager);
}

}