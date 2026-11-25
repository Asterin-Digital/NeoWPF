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
    section->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Surface));
    section->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    section->SetChild(sectionContent);
    return section;
}

Ref<NeoWPF::UI::Border> MakeGradientSwatch(
    const std::wstring& label,
    const Ref<NeoWPF::UI::Brush>& brush)
{
    auto swatch = New<NeoWPF::UI::Border>();
    swatch->SetWidth(180);
    swatch->SetHeight(72);
    swatch->SetCornerRadius(NeoWPF::UI::CornerRadius(6));
    swatch->SetBorderThickness(1);
    swatch->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    swatch->SetBackgroundBrush(brush);

    auto text = MakeText(label, 13, 600);
    text->SetForeground(GetPaletteBrush(PaletteBrush::Surface));
    swatch->SetChild(text);
    return swatch;
}

Ref<NeoWPF::UI::LinearGradientBrush> MakeLinearGradient()
{
    auto brush = New<NeoWPF::UI::LinearGradientBrush>();
    brush->SetStartPoint({0.0, 0.0});
    brush->SetEndPoint({1.0, 1.0});
    brush->GetGradientStops()->Add(New<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromRgb(58, 123, 213), 0.0));
    brush->GetGradientStops()->Add(New<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromRgb(138, 81, 220), 1.0));
    return brush;
}

Ref<NeoWPF::UI::RadialGradientBrush> MakeRadialGradient()
{
    auto brush = New<NeoWPF::UI::RadialGradientBrush>();
    brush->SetCenter({0.5, 0.5});
    brush->SetGradientOrigin({0.35, 0.35});
    brush->GetGradientStops()->Add(New<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromRgb(255, 190, 92), 0.0));
    brush->GetGradientStops()->Add(New<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromRgb(224, 83, 128), 1.0));
    return brush;
}

}

Ref<NeoWPF::UI::UIElement> CreateButtonPanel()
{
    auto page = MakeVerticalPanel(16);

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

    auto gradients = New<StackPanel>();
    gradients->SetOrientation(Orientation::Horizontal);
    gradients->SetSpacing(12);
    gradients->AddChild(MakeGradientSwatch(L"Linear", MakeLinearGradient()));
    gradients->AddChild(MakeGradientSwatch(L"Radial", MakeRadialGradient()));
    page->AddChild(MakeSection(L"Gradients", gradients));
    return page;
}

}