#include <Gallery/Components.h>
#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

#include <array>
#include <memory>
#include <vector>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteColor;
using NeoWPF::Core::ApplicationThemes::PaletteColor;
using NeoWPF::UI::Button;
using NeoWPF::UI::Grid;
using NeoWPF::UI::Orientation;
using NeoWPF::UI::ScrollBarVisibility;
using NeoWPF::UI::ScrollViewer;
using NeoWPF::UI::StackPanel;

struct TabDefinition {
    const wchar_t* Label;
    Ref<NeoWPF::UI::UIElement> (*CreatePanel)();
};

const std::array<TabDefinition, 2> kTabs{{
    {L"Button", &CreateButtonPanel},
    {L"TextBox", &CreateTextBoxPanel},
}};

Ref<Button> MakeTabButton(const std::wstring& label)
{
    auto button = New<Button>(label);
    button->SetPadding(NeoWPF::UI::Thickness(14, 9, 14, 9));
    return button;
}

void SetTabAppearance(const Ref<Button>& button, bool selected)
{
    button->SetStylePropertyValue(
        "Background",
        selected ? GetPaletteColor(PaletteColor::Accent) : GetPaletteColor(PaletteColor::Surface));
    button->SetStylePropertyValue(
        "Foreground",
        selected ? GetPaletteColor(PaletteColor::Surface) : GetPaletteColor(PaletteColor::Foreground));
    button->SetStylePropertyValue("BorderBrush", GetPaletteColor(PaletteColor::Border));
}

}

Ref<NeoWPF::UI::UIElement> CreateControlGalleryTabs()
{
    auto root = New<Grid>();
    root->AddRow(NeoWPF::UI::SizeMode::Auto);
    root->AddRow(NeoWPF::UI::SizeMode::Star);

    auto tabButtons = New<StackPanel>();
    tabButtons->SetOrientation(Orientation::Horizontal);
    tabButtons->SetSpacing(8);
    auto buttons = std::make_shared<std::vector<Ref<Button>>>();
    buttons->reserve(kTabs.size());

    auto panelViewer = New<ScrollViewer>();
    panelViewer->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    panelViewer->SetVerticalScrollBarVisibility(ScrollBarVisibility::Auto);
    for (std::size_t index = 0; index < kTabs.size(); ++index) {
        auto tabButton = MakeTabButton(kTabs[index].Label);
        SetTabAppearance(tabButton, index == 0);
        buttons->push_back(tabButton);
        tabButton->Click += [index, panelViewer, buttons]() {
            for (std::size_t buttonIndex = 0; buttonIndex < buttons->size(); ++buttonIndex) {
                SetTabAppearance((*buttons)[buttonIndex], buttonIndex == index);
            }
            panelViewer->SetContent(kTabs[index].CreatePanel());
        };
        tabButtons->AddChild(tabButton);
    }

    auto tabStrip = New<ScrollViewer>();
    tabStrip->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    tabStrip->SetVerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
    tabStrip->SetHeight(52);
    tabStrip->SetContent(tabButtons);

    root->AddChild(tabStrip, 0, 0);
    root->AddChild(panelViewer, 1, 0);
    panelViewer->SetContent(kTabs.front().CreatePanel());
    return root;
}

}