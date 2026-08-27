#include <memory>
#include <string>
#include <NeoWPF/NeoWPF.h>

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

    auto input = std::make_shared<TextBox>();
    input->SetWidth(480);
    input->SetHeight(48);
    input->SetPadding(12);
    input->SetFontSize(16);
    input->SetText(L"Enter your text here...");
    input->SetTextColor(0xFFEDF2F7u);
    input->SetBackgroundColor(0xFF1E293Bu);
    input->SetBorderColor(0xFF475569u);
    input->SetCornerRadius(CornerRadius(8.0f));

    card->SetChild(input);
    root->AddChild(card, 0, 0);

    return NeoWPF::Core::Application::Run();
}