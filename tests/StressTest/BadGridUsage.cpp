#include <memory>
#include <string>
#include <NeoWPF/NeoWPF.h>
#include <iostream>

int main()
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Core::Application::Current()->CreateWindow(L"NeoApp", 900, 680);

    using namespace NeoWPF;
    using namespace NeoWPF::UI;

    auto root = std::make_shared<Grid>();
    root->SetBackgroundColor(0xFF0B1220u);
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);

    auto card = std::make_shared<Border>();
    card->SetWidth(520);
    card->SetHeight(160);
    card->SetPadding(16);
    card->SetCornerRadius(CornerRadius(14.0f));
    card->SetBorderThickness(1);
    card->SetBackgroundColor(0xFF0F172Au);
    card->SetBorderColor(0xFF334155u);
    card->SetHorizontalAlignment(HorizontalAlignment::Center);
    card->SetVerticalAlignment(VerticalAlignment::Center);

    auto button = std::make_shared<Button>(L"Open Window");
    button->SetPadding(12);
    button->SetFontSize(16);
    button->SetTextColor(0xFFEDF2F7u);
    button->SetCornerRadius(CornerRadius(14.0f));
    button->SetBackgroundColor(0xFF1E293Bu);
    button->SetBorderColor(0xFF475569u);
    button->SetHorizontalAlignment(HorizontalAlignment::Center);
    button->SetVerticalAlignment(VerticalAlignment::Center);

    auto content = std::make_shared<Grid>();
    content->AddRow(SizeMode::Star, 1);
    content->AddColumn(SizeMode::Star, 1);
    content->AddChild(button, 0, 0);

    button->Click += []() {
        std::cout << "hi lol";
    };

    card->SetChild(content);
    root->AddChild(card, 0, 0);

    return NeoWPF::Core::Application::Run();
}