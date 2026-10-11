#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::UI::Checkbox;
using NeoWPF::UI::Orientation;
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

Ref<NeoWPF::UI::UIElement> CreateCheckboxPanel()
{
    auto page = MakeVerticalPanel(16);
    page->AddChild(MakeText(L"Checkbox", 25, 650));
    page->AddChild(MakeText(L"On/off choices and live state feedback", 14));

    auto status = MakeText(L"Unchecked");
    auto options = New<StackPanel>();
    options->SetOrientation(Orientation::Vertical);
    options->SetSpacing(12);

    auto notifications = New<Checkbox>();
    notifications->SetText(L"Enable notifications");
    notifications->SetChecked(false);
    notifications->Checked += [status]() {
        status->SetText(L"Checked");
    };
    notifications->Unchecked += [status]() {
        status->SetText(L"Unchecked");
    };

    auto remember = New<Checkbox>();
    remember->SetText(L"Remember my preference");
    remember->SetChecked(true);

    auto disabled = New<Checkbox>();
    disabled->SetText(L"Disabled option");
    disabled->SetChecked(true);
    disabled->SetEnabled(false);

    options->AddChild(notifications);
    options->AddChild(remember);
    options->AddChild(disabled);

    auto summary = MakeVerticalPanel(10);
    summary->AddChild(options);
    summary->AddChild(status);
    page->AddChild(MakeSection(L"Toggle states", summary));

    auto advanced = New<StackPanel>();
    advanced->SetSpacing(10);

    auto terms = New<Checkbox>();
    terms->SetText(L"I agree to the terms");
    terms->SetChecked(false);

    auto marketing = New<Checkbox>();
    marketing->SetText(L"Send product updates");
    marketing->SetChecked(true);

    advanced->AddChild(terms);
    advanced->AddChild(marketing);
    page->AddChild(MakeSection(L"Options", advanced));

    return page;
}

}