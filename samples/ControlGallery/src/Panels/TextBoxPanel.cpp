#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::UI::StackPanel;
using NeoWPF::UI::TextBlock;
using NeoWPF::UI::TextBox;

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
    section->SetBackgroundColor(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface));
    section->SetBorderColor(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    section->SetChild(sectionContent);
    return section;
}

}

Ref<NeoWPF::UI::UIElement> CreateTextBoxPanel()
{
    auto page = MakeVerticalPanel(16);
    page->AddChild(MakeText(L"TextBox", 25, 650));
    page->AddChild(MakeText(L"Text entry, placeholder, and read-only states", 14));

    auto inputContent = MakeVerticalPanel(10);
    auto input = New<TextBox>();
    input->SetWidth(360);
    input->SetHeight(40);
    input->SetPlaceholderText(L"Type something...");
    auto value = MakeText(L"No text entered");
    input->TextChanged += [value](const std::wstring& text) {
        value->SetText(text.empty() ? L"No text entered" : L"Current value: " + text);
    };
    inputContent->AddChild(MakeText(L"Editable input", 14, 600));
    inputContent->AddChild(input);
    inputContent->AddChild(value);
    page->AddChild(MakeSection(L"Text entry", inputContent));

    auto readonlyContent = MakeVerticalPanel(10);
    auto readonly = New<TextBox>();
    readonly->SetWidth(360);
    readonly->SetHeight(40);
    readonly->SetText(L"This value is read-only");
    readonly->SetReadOnly(true);
    readonlyContent->AddChild(MakeText(L"Read-only value", 14, 600));
    readonlyContent->AddChild(readonly);
    page->AddChild(MakeSection(L"Read-only", readonlyContent));
    return page;
}

}