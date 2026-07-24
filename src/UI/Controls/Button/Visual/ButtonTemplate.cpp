#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <memory>

namespace NeoWPF::UI {

ButtonTemplate::ButtonTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillColor(0xFFEEEEEEu);
    background->SetBorderColor(0xFF707070u);
    background->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            if (auto button = dynamic_cast<Button*>(element)) {
                templateVisual->SetFillColor(button->GetBackgroundColor().ToArgb());
                templateVisual->SetBorderColor(button->GetBorderColor().ToArgb());
            } else {
                templateVisual->SetFillColor(0xFFEEEEEEu);
                templateVisual->SetBorderColor(0xFF707070u);
            }
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(normalState);

    auto mouseOverState = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOverState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFBEE6FDu);
            templateVisual->SetBorderColor(0xFF3C7FB1u);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFC4E5F6u);
            templateVisual->SetBorderColor(0xFF3C7FB1u);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(pressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(0xFFF4F4F4u);
            templateVisual->SetBorderColor(0xFF707070u);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(disabledState);

    visualStateManager->AddGroup(commonStates);
    SetVisualStateManager(visualStateManager);
}

}
