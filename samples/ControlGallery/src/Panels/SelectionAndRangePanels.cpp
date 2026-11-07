#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::UI::ComboBox;
using NeoWPF::UI::ListBox;
using NeoWPF::UI::ProgressBar;
using NeoWPF::UI::RadioButton;
using NeoWPF::UI::Slider;
using NeoWPF::UI::StackPanel;
using NeoWPF::UI::TextBlock;

Ref<TextBlock> MakeText(const std::wstring& text, int fontSize = 14, int fontWeight = 400)
{
    auto label = New<TextBlock>(text);
    label->SetFontSize(fontSize);
    label->SetFontWeight(fontWeight);
    return label;
}

Ref<StackPanel> MakeVerticalPanel(double spacing = 12)
{
    auto panel = New<StackPanel>();
    panel->SetSpacing(spacing);
    return panel;
}

Ref<NeoWPF::UI::Border> MakeSection(const std::wstring& title,
                                    const Ref<NeoWPF::UI::UIElement>& content)
{
    auto sectionContent = MakeVerticalPanel(12);
    sectionContent->AddChild(MakeText(title, 17, 600));
    sectionContent->AddChild(content);

    auto section = New<NeoWPF::UI::Border>();
    section->SetPadding(18);
    section->SetBorderThickness(1);
    section->SetCornerRadius(NeoWPF::UI::CornerRadius(4));
    section->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Surface));
    section->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    section->SetChild(sectionContent);
    return section;
}

}

Ref<NeoWPF::UI::UIElement> CreateSelectionPanel()
{
    auto page = MakeVerticalPanel(16);

    auto choices = MakeVerticalPanel(8);
    auto standard = New<RadioButton>();
    standard->SetText(L"Standard delivery");
    standard->SetGroupName(L"delivery");
    standard->SetChecked(true);
    auto express = New<RadioButton>();
    express->SetText(L"Express delivery");
    express->SetGroupName(L"delivery");
    choices->AddChild(standard);
    choices->AddChild(express);
    page->AddChild(MakeSection(L"Radio buttons", choices));

    auto destinations = New<ComboBox>();
    destinations->SetWidth(300);
    destinations->AddItem(New<TextBlock>(L"Seattle"));
    destinations->AddItem(New<TextBlock>(L"Portland"));
    destinations->AddItem(New<TextBlock>(L"Vancouver"));
    destinations->SetSelectedIndex(0);
    page->AddChild(MakeSection(L"Combo box", destinations));

    auto recentItems = New<ListBox>();
    recentItems->SetHeight(132);
    recentItems->AddItem(New<TextBlock>(L"Design system"));
    recentItems->AddItem(New<TextBlock>(L"Control themes"));
    recentItems->AddItem(New<TextBlock>(L"Input patterns"));
    recentItems->AddItem(New<TextBlock>(L"Layout samples"));
    recentItems->SetSelectedIndex(1);
    page->AddChild(MakeSection(L"List box", recentItems));
    return page;
}

Ref<NeoWPF::UI::UIElement> CreateRangePanel()
{
    auto page = MakeVerticalPanel(16);

    auto rangeContent = MakeVerticalPanel(12);
    auto valueLabel = MakeText(L"68% complete", 14, 600);
    auto slider = New<Slider>();
    slider->SetWidth(360);
    slider->SetHeight(36);
    slider->SetMinimum(0.0);
    slider->SetMaximum(100.0);
    slider->SetValue(68.0);

    auto progress = New<ProgressBar>();
    progress->SetWidth(360);
    progress->SetHeight(16);
    progress->SetMaximum(100.0);
    progress->SetValue(slider->GetValue());
    slider->ValueChanged += [progress, valueLabel](double value) {
        progress->SetValue(value);
        valueLabel->SetText(std::to_wstring(static_cast<int>(value)) + L"% complete");
    };

    rangeContent->AddChild(valueLabel);
    rangeContent->AddChild(slider);
    rangeContent->AddChild(progress);
    page->AddChild(MakeSection(L"Slider and progress bar", rangeContent));

    auto activity = MakeVerticalPanel(10);
    auto indeterminate = New<ProgressBar>();
    indeterminate->SetWidth(360);
    indeterminate->SetHeight(16);
    indeterminate->SetIsIndeterminate(true);
    activity->AddChild(MakeText(L"Background synchronization"));
    activity->AddChild(indeterminate);
    page->AddChild(MakeSection(L"Indeterminate progress", activity));
    return page;
}

}