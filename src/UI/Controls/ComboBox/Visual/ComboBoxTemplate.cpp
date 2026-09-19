#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <memory>

namespace NeoWPF::UI {

ComboBoxTemplate::ComboBoxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillColor(0xFFFFFFFFu);
    background->SetBorderColor(0xFF707070u);
    background->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    auto popup = std::make_shared<Popup>();
    auto popupBorder = std::make_shared<Border>();
    popupBorder->SetBorderThickness(1);
    popupBorder->SetBorderColor(0xFFAAAAAAu);
    popupBorder->SetBackgroundColor(0xFFF0F0F0u);

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_ContentHost", contentHost);
    AddTemplateChild("PART_Popup", popup);
    AddTemplateChild("PART_PopupBorder", popupBorder);

    auto manager = std::make_shared<Styling::VisualStateManager>();
    auto commonStates = std::make_shared<Styling::VisualStateGroup>("CommonStates");

    auto normal = std::make_shared<Styling::VisualState>("Normal");
    normal->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillColor(0xFFFFFFFFu);
            part->SetBorderColor(0xFF707070u);
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(normal);

    auto mouseOver = std::make_shared<Styling::VisualState>("MouseOver");
    mouseOver->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillColor(0xFFF5FAFFu);
            part->SetBorderColor(0xFF3C7FB1u);
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(mouseOver);

    auto open = std::make_shared<Styling::VisualState>("Open");
    open->AddSetter("PART_Background", [](UIElement* element, Visual* visual) {
        if (auto part = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            part->SetFillColor(0xFFEAF4FFu);
            part->SetBorderColor(0xFF3C7FB1u);
        }
        if (element) element->InvalidateVisual();
    });
    commonStates->AddState(open);

    manager->AddGroup(commonStates);
    SetVisualStateManager(manager);
}

}
