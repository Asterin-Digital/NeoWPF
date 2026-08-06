#include <NeoWPF/NeoWPF.h>
#include <array>
#include <cstdint>
#include <string>

using namespace NeoWPF;

int main()
{
    Core::Application::Initialize();

    Core::Application::SetResource("Button.BackgroundColor", std::uint32_t(0xFFB71C1Cu));
    Core::Application::SetResource("Button.TextColor", std::uint32_t(0xFFFFFFFFu));
    Core::Application::SetResource("Button.Padding", 12);
    Core::Application::SetResource("TextBlock.TextColor", std::uint32_t(0xFFFFEBEEu));
    Core::Application::SetResource("TextBox.BackgroundColor", std::uint32_t(0xFF2C0F0Fu));
    Core::Application::SetResource("TextBox.TextColor", std::uint32_t(0xFFFFF3F3u));
    Core::Application::SetResource("TextBox.BorderColor", std::uint32_t(0xFFEF5350u));
    Core::Application::SetResource("TextBox.CaretColor", std::uint32_t(0xFFFFCDD2u));
    Core::Application::SetResource("Border.BackgroundColor", std::uint32_t(0xFF1E0C0Cu));
    Core::Application::SetResource("Border.BorderColor", std::uint32_t(0xFFEF5350u));

    auto window = Core::Application::Current()->CreateWindow(L"Saphire Styling Test", 1120, 760);

    auto root = std::make_shared<UI::Grid>();
    root->AddRow(UI::SizeMode::Auto, 64);
    root->AddRow(UI::SizeMode::Star, 1);
    root->AddColumn(UI::SizeMode::Fixed, 300);
    root->AddColumn(UI::SizeMode::Star, 1);

    window.SetContent(root);

    auto sidebar = std::make_shared<UI::Border>();
    sidebar->SetWidth(300);
    sidebar->SetPadding(18);
    sidebar->SetCornerRadius(UI::CornerRadius(18.0f));
    sidebar->SetBorderThickness(2);
    sidebar->SetBackgroundColor(0xFF2C0F0Fu);
    sidebar->SetBorderColor(0xFFEF5350u);
    sidebar->SetHorizontalAlignment(UI::HorizontalAlignment::Left);
    sidebar->SetVerticalAlignment(UI::VerticalAlignment::Top);

    auto sidebarContent = std::make_shared<UI::Grid>();
    sidebarContent->AddRow(UI::SizeMode::Auto, 1);
    sidebarContent->AddRow(UI::SizeMode::Star, 1);
    sidebarContent->AddColumn(UI::SizeMode::Star, 1);

    auto inputBox = std::make_shared<UI::TextBox>();
    inputBox->SetText(L"Styled textbox");
    inputBox->SetWidth(250);
    inputBox->SetHeight(42);
    inputBox->SetPadding(10);
    inputBox->SetCornerRadius(UI::CornerRadius(10.0f));
    inputBox->SetBackgroundColor(0xFF3F1212u);
    inputBox->SetBorderColor(0xFFFF8A80u);
    inputBox->SetTextColor(0xFFFFF3F3u);

    auto buttonStack = std::make_shared<UI::StackPanel>();
    buttonStack->SetWidth(250);

    const std::wstring labels[] = {
        L"Primary action",
        L"Secondary action",
        L"Danger action",
        L"Accent action",
        L"Confirm action"
    };

    for (const auto& label : labels) {
        auto button = std::make_shared<UI::Button>(label);
        button->SetHeight(42);
        button->SetWidth(250);
        button->SetPadding(10);
        button->SetCornerRadius(UI::CornerRadius(10.0f));
        button->SetBackgroundColor(0xFFB71C1Cu);
        button->SetTextColor(0xFFFFFFFFu);
        button->SetBorderColor(0xFFFFB0B0u);
        buttonStack->AddChild(button);
    }

    sidebarContent->AddChild(inputBox, 0, 0);
    sidebarContent->AddChild(buttonStack, 1, 0);
    sidebar->SetChild(sidebarContent);
    root->AddChild(sidebar, 0, 0, 2, 1);

    auto title = std::make_shared<UI::TextBlock>(L"styling test!");
    title->SetFontSize(30);
    title->SetTextColor(0xFFFFEBEEu);
    title->SetHorizontalAlignment(UI::HorizontalAlignment::Left);
    title->SetVerticalAlignment(UI::VerticalAlignment::Center);
    root->AddChild(title, 0, 1);

    auto contentCard = std::make_shared<UI::Border>();
    contentCard->SetPadding(18);
    contentCard->SetCornerRadius(UI::CornerRadius(18.0f));
    contentCard->SetBorderThickness(2);
    contentCard->SetBackgroundColor(0xFF1E0C0Cu);
    contentCard->SetBorderColor(0xFFEF5350u);
    contentCard->SetHorizontalAlignment(UI::HorizontalAlignment::Stretch);
    contentCard->SetVerticalAlignment(UI::VerticalAlignment::Stretch);
    contentCard->SetWidthMode(UI::LayoutSizeMode::Stretch);
    contentCard->SetHeightMode(UI::LayoutSizeMode::Stretch);

    auto listHost = std::make_shared<UI::TextBox>();
    listHost->SetText(L"");
    listHost->SetAcceptsReturn(true);
    listHost->SetTextWrapping(UI::TextWrapping::Wrap);
    listHost->SetPadding(12);
    listHost->SetCornerRadius(UI::CornerRadius(12.0f));
    listHost->SetBackgroundColor(0xFF2C0F0Fu);
    listHost->SetBorderColor(0xFFFF8A80u);
    listHost->SetTextColor(0xFFFFF3F3u);
    listHost->SetHorizontalAlignment(UI::HorizontalAlignment::Stretch);
    listHost->SetVerticalAlignment(UI::VerticalAlignment::Stretch);
    listHost->SetWidthMode(UI::LayoutSizeMode::Stretch);
    listHost->SetHeightMode(UI::LayoutSizeMode::Stretch);

    std::wstring items;
    for (int i = 1; i <= 70; ++i) {
        items += L"Text block #" + std::to_wstring(i) + L"\n";
    }
    listHost->SetText(items);

    contentCard->SetChild(listHost);
    root->AddChild(contentCard, 1, 1);

    return Core::Application::Run();
}