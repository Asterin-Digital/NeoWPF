#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <memory>

namespace NeoWPF::UI {
namespace {
using NeoWPF::Core::ApplicationThemes::GetPaletteColor;
using NeoWPF::Core::ApplicationThemes::PaletteColor;
}

ComboBoxItemTemplate::ComboBoxItemTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillColor(0x00000000u);
    background->SetBorderColor(0x00000000u);
    background->SetBorderThickness(0);

    auto selectionOverlay = std::make_shared<Templates::TemplateVisual>("PART_SelectionOverlay");
    selectionOverlay->SetFillColor(GetPaletteColor(PaletteColor::AccentPressed).ToArgb());
    selectionOverlay->SetOpacity(0.0f);

    auto highlightOverlay = std::make_shared<Templates::TemplateVisual>("PART_HighlightOverlay");
    highlightOverlay->SetFillColor(GetPaletteColor(PaletteColor::AccentHover).ToArgb());
    highlightOverlay->SetOpacity(0.0f);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_SelectionOverlay", selectionOverlay);
    AddTemplateChild("PART_HighlightOverlay", highlightOverlay);
    AddTemplateChild("PART_ContentHost", contentHost);

    auto visualStateManager = std::make_shared<Styling::VisualStateManager>();

    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");
    auto normalState = std::make_shared<Styling::VisualState>("Normal");
    normalState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillColor(0x00000000u);
            part->SetBorderColor(0x00000000u);
            part->SetBorderThickness(0);
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(normalState);

    auto mouseOverState = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOverState->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillColor(GetPaletteColor(PaletteColor::AccentHover).ToArgb());
            part->SetBorderColor(GetPaletteColor(PaletteColor::Focus).ToArgb());
            part->SetBorderThickness(1);
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(mouseOverState);
    visualStateManager->AddGroup(commonStates);

    auto selectionStates = std::make_shared<Styling::VisualStateGroup>("SelectionStates");
    auto unselectedState = std::make_shared<Styling::VisualState>("Unselected");
    unselectedState->AddSetter("PART_SelectionOverlay", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) part->SetOpacity(0.0f);
        if (element) element->InvalidateVisual();
    });
    selectionStates->AddState(unselectedState);

    auto selectedState = std::make_shared<Styling::VisualState>("Selected");
    selectedState->AddSetter("PART_SelectionOverlay", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) part->SetOpacity(1.0f);
        if (element) element->InvalidateVisual();
    });
    selectionStates->AddState(selectedState);
    visualStateManager->AddGroup(selectionStates);

    auto highlightStates = std::make_shared<Styling::VisualStateGroup>("HighlightStates");
    auto notHighlightedState = std::make_shared<Styling::VisualState>("NotHighlighted");
    notHighlightedState->AddSetter("PART_HighlightOverlay", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) part->SetOpacity(0.0f);
        if (element) element->InvalidateVisual();
    });
    highlightStates->AddState(notHighlightedState);

    auto highlightedState = std::make_shared<Styling::VisualState>("Highlighted");
    highlightedState->AddSetter("PART_HighlightOverlay", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) part->SetOpacity(1.0f);
        if (element) element->InvalidateVisual();
    });
    highlightStates->AddState(highlightedState);
    visualStateManager->AddGroup(highlightStates);

    SetVisualStateManager(visualStateManager);
}

}
