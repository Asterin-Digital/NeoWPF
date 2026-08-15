#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

TextBoxTemplate::TextBoxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto border = std::make_shared<Templates::TemplateVisual>("PART_Border");
    border->SetFillColor(0xFFFFFFFFu);
    border->SetBorderColor(0xFFABADB3u);
    border->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Border", border);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            uint32_t bgColor = 0xFFFFFFFFu;
            uint32_t borderColor = 0xFFABADB3u;
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                bgColor = textBox->GetBackgroundColor().ToArgb();
                borderColor = textBox->GetBorderColor().ToArgb();
            }
            
            templateVisual->SetFillColor(bgColor);
            templateVisual->SetBorderColor(borderColor);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(normalState);

    auto focusedState = std::make_shared<Styling::VisualState>("Focused");
    focusedState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            uint32_t bgColor = 0xFFFFFFFFu;
            uint32_t borderColor = 0xFF569DE5u;
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                bgColor = textBox->GetBackgroundColor().ToArgb();
                borderColor = textBox->GetBorderColor().ToArgb();
            }
            
            templateVisual->SetFillColor(bgColor);
            templateVisual->SetBorderColor(borderColor);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(focusedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            uint32_t bgColor = 0xFFF0F0F0u;
            uint32_t borderColor = 0xFFABADB3u;
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                bgColor = textBox->GetBackgroundColor().ToArgb();
                borderColor = textBox->GetBorderColor().ToArgb();
            }
            
            templateVisual->SetFillColor(bgColor);
            templateVisual->SetBorderColor(borderColor);
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
