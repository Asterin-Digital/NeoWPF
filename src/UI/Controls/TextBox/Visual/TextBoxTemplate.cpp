#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

namespace {
using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;

}

TextBoxTemplate::TextBoxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto border = std::make_shared<Templates::TemplateVisual>("PART_Border");
    border->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface));
    border->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    border->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Border", border);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            std::shared_ptr<Brush> backgroundBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface);
            std::shared_ptr<Brush> borderBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border);
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                backgroundBrush = textBox->GetBackgroundBrush();
                borderBrush = textBox->GetBorderBrush();
            }
            
            templateVisual->SetFillBrush(backgroundBrush);
            templateVisual->SetBorderBrush(borderBrush);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(normalState);

    auto focusedState = std::make_shared<Styling::VisualState>("Focused");
    focusedState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            std::shared_ptr<Brush> backgroundBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface);
            std::shared_ptr<Brush> borderBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Focus);
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                backgroundBrush = textBox->GetBackgroundBrush();
                borderBrush = textBox->GetBorderBrush();
            }
            
            templateVisual->SetFillBrush(backgroundBrush);
            templateVisual->SetBorderBrush(borderBrush);
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(focusedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Border", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            std::shared_ptr<Brush> backgroundBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::DisabledSurface);
            std::shared_ptr<Brush> borderBrush = GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border);
            
            if (auto* textBox = dynamic_cast<TextBox*>(element)) {
                backgroundBrush = textBox->GetBackgroundBrush();
                borderBrush = textBox->GetBorderBrush();
            }
            
            templateVisual->SetFillBrush(backgroundBrush);
            templateVisual->SetBorderBrush(borderBrush);
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
