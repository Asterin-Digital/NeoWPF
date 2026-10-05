#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <memory>

namespace NeoWPF::UI {

namespace {

NeoWPF::UI::Color GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor paletteColor)
{
    NeoWPF::UI::Color color;
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().TryGetPaletteColor(paletteColor, color);
    return color;
}

}

ButtonTemplate::ButtonTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Surface).ToArgb());
    background->SetBorderColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Border).ToArgb());
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
            templateVisual->SetFillColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::AccentHover).ToArgb());
            templateVisual->SetBorderColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Accent).ToArgb());
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(mouseOverState);

    auto pressedState = std::make_shared<Styling::VisualState>("Pressed");
    pressedState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::AccentPressed).ToArgb());
            templateVisual->SetBorderColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Accent).ToArgb());
        }
        if (element) {
            element->InvalidateVisual();
        }
    });
    commonStates->AddState(pressedState);

    auto disabledState = std::make_shared<Styling::VisualState>("Disabled");
    disabledState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            templateVisual->SetFillColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::DisabledSurface).ToArgb());
            templateVisual->SetBorderColor(GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Border).ToArgb());
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
