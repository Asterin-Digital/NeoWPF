#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::Core::ApplicationThemes::ThemeManager;
using NeoWPF::UI::Button;
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

Ref<Button> MakeActionButton(const std::wstring& text)
{
    auto button = New<Button>(text);
    button->SetPadding(NeoWPF::UI::Thickness(14, 9, 14, 9));
    return button;
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
    section->SetBackgroundColor(GetPaletteBrush(PaletteBrush::Surface));
    section->SetBorderColor(GetPaletteBrush(PaletteBrush::Border));
    section->SetChild(sectionContent);
    return section;
}

}

Ref<NeoWPF::UI::UIElement> CreateButtonPanel()
{
    auto page = MakeVerticalPanel(16);
    page->AddChild(MakeText(L"Button", 25, 650));
    page->AddChild(MakeText(L"Actions, states, and theme response", 14));

    auto actions = New<StackPanel>();
    actions->SetOrientation(Orientation::Horizontal);
    actions->SetSpacing(10);
    auto status = MakeText(L"Ready", 14);

    auto saveButton = MakeActionButton(L"Save changes");
    saveButton->Click += [status]() { status->SetText(L"Changes saved"); };
    auto cancelButton = MakeActionButton(L"Cancel");
    cancelButton->Click += [status]() { status->SetText(L"Action cancelled"); };
    actions->AddChild(saveButton);
    actions->AddChild(cancelButton);

    auto actionContent = MakeVerticalPanel(12);
    actionContent->AddChild(actions);
    actionContent->AddChild(status);
    page->AddChild(MakeSection(L"Actions", actionContent));

    auto states = New<StackPanel>();
    states->SetOrientation(Orientation::Horizontal);
    states->SetSpacing(10);
    states->AddChild(MakeActionButton(L"Enabled"));
    auto disabledButton = MakeActionButton(L"Disabled");
    disabledButton->SetEnabled(false);
    states->AddChild(disabledButton);
    page->AddChild(MakeSection(L"Enabled and disabled", states));

    auto themeContent = MakeVerticalPanel(12);
    auto themeButton = MakeActionButton(L"Switch light / dark theme");
    themeButton->Click += []() {
        auto& themes = ThemeManager::Instance();
        themes.SetActiveTheme(themes.GetActiveThemeName() == "Light Mint" ? "Dark Mint" : "Light Mint");
    };
    themeContent->AddChild(themeButton);
    themeContent->AddChild(MakeText(L"The gallery controls follow the active application theme."));
    page->AddChild(MakeSection(L"Theme", themeContent));
    return page;
}

}