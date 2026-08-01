#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <memory>

namespace NeoWPF::UI {

CheckboxTemplate::CheckboxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto box = std::make_shared<Templates::TemplateVisual>("PART_Box");
    box->SetFillColor(0xFFFFFFFFu);
    box->SetBorderColor(0xFF202020u);
    box->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Box", box);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            const auto fill = element->GetResourceValue("Checkbox.BoxColor", NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
            const auto border = element->GetResourceValue("Checkbox.BorderColor", NeoWPF::UI::Color::FromArgb(0xFF202020u));
            templateVisual->SetFillColor(fill.ToArgb());
            templateVisual->SetBorderColor(border.ToArgb());
        }
    });
    commonStates->AddState(normalState);

    auto checkedState = std::make_shared<Styling::VisualState>("Checked");
    checkedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            const auto fill = element->GetResourceValue("Checkbox.BoxColor", NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
            const auto border = element->GetResourceValue("Checkbox.BorderColor", NeoWPF::UI::Color::FromArgb(0xFF202020u));
            templateVisual->SetFillColor(fill.ToArgb());
            templateVisual->SetBorderColor(border.ToArgb());
        }
    });
    commonStates->AddState(checkedState);

    auto mouseOverState = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOverState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            const auto fill = element->GetResourceValue("Checkbox.VisualState.MouseOver.Background", NeoWPF::UI::Color::FromArgb(0xFFF2F9FFu));
            const auto border = element->GetResourceValue("Checkbox.VisualState.MouseOver.Border", NeoWPF::UI::Color::FromArgb(0xFF202020u));
            templateVisual->SetFillColor(fill.ToArgb());
            templateVisual->SetBorderColor(border.ToArgb());
        }
    });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            const auto fill = element->GetResourceValue("Checkbox.VisualState.Pressed.Background", NeoWPF::UI::Color::FromArgb(0xFFF2F9FFu));
            const auto border = element->GetResourceValue("Checkbox.VisualState.Pressed.Border", NeoWPF::UI::Color::FromArgb(0xFF202020u));
            templateVisual->SetFillColor(fill.ToArgb());
            templateVisual->SetBorderColor(border.ToArgb());
        }
    });
    commonStates->AddState(pressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            const auto fill = element->GetResourceValue("Checkbox.VisualState.Disabled.Background", NeoWPF::UI::Color::FromArgb(0xFF2A2A2Au));
            const auto border = element->GetResourceValue("Checkbox.VisualState.Disabled.Border", NeoWPF::UI::Color::FromArgb(0xFF404040u));
            templateVisual->SetFillColor(fill.ToArgb());
            templateVisual->SetBorderColor(border.ToArgb());
        }
    });
    commonStates->AddState(disabledState);

    visualStateManager->AddGroup(commonStates);
    SetVisualStateManager(visualStateManager);
}

}
