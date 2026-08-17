#include <NeoWPF/NeoWPF.h>

int main() {
using namespace NeoWPF::Diagnostics;

SetGlobalLevel(Level::Info);

SetLevel(Category::Visual,             Level::Off);
SetLevel(Category::GridLayout,         Level::Off);
SetLevel(Category::ContentPresenter,   Level::Off);
SetLevel(Category::Control,            Level::Off);
SetLevel(Category::Application,        Level::Off);
SetLevel(Category::HitTest,            Level::Off);
SetLevel(Category::Virtualization,     Level::Off);
SetLevel(Category::TextBox,            Level::Off);
SetLevel(Category::TextRendering,      Level::Trace);
SetLevel(Category::Image,              Level::Off);
SetLevel(Category::Http,               Level::Off);
    NeoWPF::Core::Application::Initialize();

    auto window = NeoWPF::Core::Application::Current()->CreateWindow(L"Glyph Baseline Test", 800, 600);
    auto root = std::make_shared<NeoWPF::Grid>();
    root->SetBackgroundColor(0xFF121212u);
    root->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    root->AddRow(NeoWPF::UI::SizeMode::Auto, 0);
    root->AddRow(NeoWPF::UI::SizeMode::Auto, 0);

    auto border = std::make_shared<NeoWPF::UI::Border>();
    border->SetWidth(420);
    border->SetHeight(220);
    border->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    border->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    border->SetPadding(20);
    border->SetBorderThickness(2);
    border->SetBorderColor(0xFF555555u);
    border->SetBackgroundColor(0xFF1E1E1Eu);

    auto borderContent = std::make_shared<NeoWPF::Grid>();
    borderContent->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    borderContent->AddRow(NeoWPF::UI::SizeMode::Star, 1);
    borderContent->AddRow(NeoWPF::UI::SizeMode::Auto, 0);

    auto text = std::make_shared<NeoWPF::UI::TextBlock>(L"g");
    text->SetTextHorizontalAlignment(NeoWPF::UI::TextHorizontalAlignment::Center);
    text->SetTextVerticalAlignment(NeoWPF::UI::TextVerticalAlignment::Center);
    text->SetFontSize(28);
    text->SetFontWeight(700);
    text->SetFontStyle(NeoWPF::UI::FontStyle::Italic);
    text->SetTextColor(0xFFFFFFFFu);
    text->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);
    text->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Stretch);
    borderContent->AddChild(text, 0, 0);

    auto textbox = std::make_shared<NeoWPF::UI::TextBox>();
    textbox->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Stretch);
    textbox->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    textbox->SetHeight(40);
    textbox->SetTextWrapping(NeoWPF::UI::TextWrapping::NoWrap);
    textbox->SetBackgroundColor(0xFF2A2A2Au);
    textbox->SetBorderColor(0xFF777777u);
    textbox->SetTextColor(0xFFFFFFFFu);
    textbox->SetPlaceholderText(L"Type something...");
    textbox->SetFontSize(14);
    textbox->SetFontWeight(500);
    textbox->SetFontStyle(NeoWPF::UI::FontStyle::Normal);
    borderContent->AddChild(textbox, 1, 0);

    border->SetChild(borderContent);

    auto toggleButton = std::make_shared<NeoWPF::UI::Button>(L"Add f");
    toggleButton->SetWidth(160);
    toggleButton->SetHeight(36);
    toggleButton->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    toggleButton->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    toggleButton->SetBackgroundColor(0xFF2A2A2Au);
    toggleButton->SetBorderColor(0xFF777777u);
    toggleButton->SetTextColor(0xFFFFFFFFu);

    std::wstring currentText = L"g";
    toggleButton->Click += [text, toggleButton, currentText]() mutable {
        currentText = (currentText == L"g") ? L"gf" : L"g";
        text->SetText(currentText);
        toggleButton->SetText(currentText == L"g" ? L"Add f" : L"Remove f");

        NeoWPF::Diagnostics::TextDumpOptions dumpOptions;
        dumpOptions.includeLayout = true;
        //dumpOptions.includeColors = true;
        dumpOptions.includePadding = true;
        NeoWPF::Core::Application::DumpTextTree(dumpOptions);
    };

    root->AddChild(border, 0, 0);
    root->AddChild(toggleButton, 1, 0);
    window.SetContent(root);

    return NeoWPF::Core::Application::Run();
}