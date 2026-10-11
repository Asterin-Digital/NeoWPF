#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <memory>

namespace NeoWPF::UI {
namespace {
using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
}

ComboBoxTemplate::ComboBoxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillBrush(GetPaletteBrush(PaletteBrush::Surface));
    background->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    background->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetBorderThickness(0);

    auto popup = std::make_shared<Popup>();
    auto popupBorder = std::make_shared<Border>();

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_ContentHost", contentHost);
    AddTemplateChild("PART_Popup", popup);
    AddTemplateChild("PART_PopupBorder", popupBorder);

    auto manager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normal = std::make_shared<Styling::VisualState>("Normal");
    normal->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillBrush(GetPaletteBrush(PaletteBrush::Surface));
            part->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(normal);

    auto mouseOver = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOver->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillBrush(GetPaletteBrush(PaletteBrush::AccentHover));
            part->SetBorderBrush(GetPaletteBrush(PaletteBrush::Focus));
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(mouseOver);

    auto open = std::make_shared<Styling::VisualState>("Open");
    open->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillBrush(GetPaletteBrush(PaletteBrush::AccentPressed));
            part->SetBorderBrush(GetPaletteBrush(PaletteBrush::Focus));
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(open);

    manager->AddGroup(commonStates);
    SetVisualStateManager(manager);
}

}
