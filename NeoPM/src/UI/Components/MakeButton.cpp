#include <NeoPM/UI/Components/MakeButton.h>
#include <NeoPM/UI/Components/Theme.h>

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Components {

using namespace NeoWPF::UI;
using std::wstring;

namespace {

class ButtonTemplate final : public Templates::ControlTemplate {
public:
    ButtonTemplate()
    {
        SetVisualTree(New<Visual>());

        auto background = New<Templates::TemplateVisual>("PART_Background");
        background->SetBorderThickness(1);
        background->SetCornerRadius(CornerRadius(8));

        auto contentHost = New<Templates::TemplateVisual>("PART_ContentHost");
        AddTemplateChild("PART_Background", background);
        AddTemplateChild("PART_ContentHost", contentHost);

        auto stateManager = New<Styling::VisualStateManager>();
        auto commonStates = New<Styling::VisualStateGroup>("CommonStates");
        AddState(*commonStates, "Normal", true, Theme::ButtonBackground, Theme::ButtonBorder);
        AddState(*commonStates, "MouseOver", false, Theme::ButtonHoverBackground, Theme::ButtonHoverBorder);
        AddState(*commonStates, "Pressed", false, Theme::ButtonPressedBackground, Theme::ButtonPressedBorder);
        AddState(*commonStates, "Disabled", false, Theme::ButtonDisabledBackground, Theme::ButtonDisabledBorder);
        stateManager->AddGroup(commonStates);
        SetVisualStateManager(stateManager);
    }

private:
    static void AddState(
        Styling::VisualStateGroup& group,
        const char* name,
        bool useButtonColors,
        std::uint32_t backgroundColor,
        std::uint32_t borderColor)
    {
        auto state = New<Styling::VisualState>(name);
        state->AddSetter("PART_Background", [useButtonColors, backgroundColor, borderColor](UIElement* element, Visual* visual) {
            if (auto templateVisual = dynamic_cast<Templates::TemplateVisual*>(visual)) {
            if (useButtonColors) {
                    if (auto button = dynamic_cast<Button*>(element)) {
                        templateVisual->SetFillColor(button->GetBackgroundColor().ToArgb());
                        templateVisual->SetBorderColor(button->GetBorderColor().ToArgb());
                    }
                } else {
                    templateVisual->SetFillColor(backgroundColor);
                    templateVisual->SetBorderColor(borderColor);
                }
            }
            if (element) {
                element->InvalidateVisual();
            }
        });
        group.AddState(state);
    }
};

void SetButtonSelected(
    const Ref<Button>& button,
    bool selected)
{
    if (!button) {
        return;
    }

    button->SetBackgroundColor(selected ? Theme::ButtonSelectedBackground : Theme::ButtonBackground);
    button->SetBorderColor(selected ? Theme::ButtonSelectedBorder : Theme::ButtonBorder);
    button->SetTextColor(selected ? Theme::ButtonSelectedText : Theme::ButtonText);
}

void EnableSelectionToggle(const Ref<Button>& button)
{
    auto selected = New<bool>(false);
    button->Click += [button, selected]() {
        *selected = !*selected;
        SetButtonSelected(button, *selected);
    };
}

}

Ref<Button> MakeButton(
    const wstring& text,
    bool isSelectable)
{
    auto button = New<Button>(text);
    button->SetTemplate(New<ButtonTemplate>());
    button->ApplyTemplate();
    button->SetWidth(208);
    button->SetHeight(42);
    button->SetHorizontalAlignment(HorizontalAlignment::Center);
    button->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    button->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    button->SetFontSize(15);
    button->SetFontWeight(600);
    button->SetPadding(Thickness(16, 0, 16, 0));
    button->SetCornerRadius(CornerRadius(8));
    SetButtonSelected(button, false);
    if (isSelectable) {
        EnableSelectionToggle(button);
    }
    return button;
}

}
