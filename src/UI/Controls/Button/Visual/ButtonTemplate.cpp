#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <memory>

namespace NeoWPF::UI {

namespace {
using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;

}

ButtonTemplate::ButtonTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface));
    background->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    background->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            if (auto button = dynamic_cast<Button*>(element)) {
                templateVisual->SetFillBrush(button->GetBackgroundBrush());
                templateVisual->SetBorderBrush(button->GetBorderBrush());
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
            templateVisual->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::AccentHover));
            templateVisual->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent));
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::AccentPressed));
            templateVisual->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent));
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(pressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::DisabledSurface));
            templateVisual->SetBorderBrush(GetPaletteBrush(NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
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
