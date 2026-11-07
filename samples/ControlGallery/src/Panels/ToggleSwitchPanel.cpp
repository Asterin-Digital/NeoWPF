#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::UI::Orientation;
using NeoWPF::UI::StackPanel;
using NeoWPF::UI::TextBlock;
using NeoWPF::UI::ToggleSwitch;

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

Ref<NeoWPF::UI::Border> MakeSection(const std::wstring& title, const Ref<NeoWPF::UI::UIElement>& content)
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

Ref<NeoWPF::UI::UIElement> CreateToggleSwitchPanel()
{
    auto page = MakeVerticalPanel(16);

    auto settings = MakeVerticalPanel(10);
    auto wifi = New<ToggleSwitch>();
    wifi->SetContentText(L"Wi-Fi");
    wifi->SetChecked(true);

    auto automaticUpdates = New<ToggleSwitch>();
    automaticUpdates->SetContentText(L"Automatic updates");
    automaticUpdates->SetChecked(false);

    settings->AddChild(wifi);
    settings->AddChild(automaticUpdates);
    page->AddChild(MakeSection(L"Settings", settings));

    auto states = MakeVerticalPanel(10);
    auto enabled = New<ToggleSwitch>();
    enabled->SetContentText(L"Stay signed in");
    enabled->SetChecked(true);

    auto disabled = New<ToggleSwitch>();
    disabled->SetContentText(L"Unavailable setting");
    disabled->SetEnabled(false);

    states->AddChild(enabled);
    states->AddChild(disabled);
    page->AddChild(MakeSection(L"Checked and disabled states", states));

    auto interaction = MakeVerticalPanel(10);
    auto backgroundSync = New<ToggleSwitch>();
    backgroundSync->SetContentText(L"Background sync");
    auto status = MakeText(L"Background sync is off");
    backgroundSync->Checked += [status]() {
        status->SetText(L"Background sync is on");
    };
    backgroundSync->Unchecked += [status]() {
        status->SetText(L"Background sync is off");
    };
    interaction->AddChild(backgroundSync);
    interaction->AddChild(status);
    interaction->AddChild(MakeText(L"SetContentText assigns the label, and SetChecked sets the initial state."));
    page->AddChild(MakeSection(L"Usage", interaction));

    return page;
}

}