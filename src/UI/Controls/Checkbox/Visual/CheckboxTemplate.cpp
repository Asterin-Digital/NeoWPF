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
            templateVisual->SetFillColor(0xFFFFFFFFu);
            templateVisual->SetBorderColor(0xFF202020u);
        }
    });
    commonStates->AddState(normalState);

    auto checkedState = std::make_shared<Styling::VisualState>("Checked");
    checkedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFFFFFFFu);
            templateVisual->SetBorderColor(0xFF202020u);
        }
    });
    commonStates->AddState(checkedState);

    auto mouseOverState = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOverState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFF2F9FFu);
            templateVisual->SetBorderColor(0xFF202020u);
        }
    });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFF2F9FFu);
            templateVisual->SetBorderColor(0xFF202020u);
        }
    });
    commonStates->AddState(pressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Box", [](UIElement* element, Visual* visual) {
        (void)element;
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFF2A2A2Au);
            templateVisual->SetBorderColor(0xFF404040u);
        }
    });
    commonStates->AddState(disabledState);

    visualStateManager->AddGroup(commonStates);
    SetVisualStateManager(visualStateManager);
}

}
