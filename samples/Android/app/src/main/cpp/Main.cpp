#include <NeoWPF/NeoWPF.h>

#include <memory>

int main()
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::SetLevel(
        NeoWPF::Diagnostics::Category::TextRendering,
        NeoWPF::Diagnostics::Level::Debug);
    auto windowHost = std::make_shared<NeoWPF::UI::Window>();
    windowHost->SetTitle(L"NeoWPF Android");

    using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::UI::Border;
    using NeoWPF::UI::Grid;
    using NeoWPF::UI::SizeMode;
    using NeoWPF::UI::StackPanel;
    using NeoWPF::UI::TextBlock;
    using NeoWPF::UI::VerticalAlignment;

    const auto makeText = [](const std::wstring& value, int fontSize, int fontWeight) {
        auto text = std::make_shared<TextBlock>(value);
        text->SetForeground(GetPaletteBrush(PaletteBrush::Foreground));
        text->SetFontSize(fontSize);
        text->SetFontWeight(fontWeight);
        return text;
    };

    auto content = std::make_shared<StackPanel>();
    content->SetSpacing(28);
    content->SetMargin(44);
    content->SetVerticalAlignment(VerticalAlignment::Center);

    content->AddChild(makeText(L"Hello from NeoWPF", 52, 700));
    content->AddChild(makeText(
        L"A native Android window, drawn with Vulkan.", 28, 400));

    auto accent = std::make_shared<Border>();
    accent->SetWidth(72);
    accent->SetHeight(6);
    accent->SetCornerRadius(NeoWPF::UI::CornerRadius(3));
    accent->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Accent));
    content->AddChild(accent);

    auto status = std::make_shared<StackPanel>();
    status->SetSpacing(12);
    status->AddChild(makeText(L"RENDERER", 20, 600));
    status->AddChild(makeText(L"Vulkan surface ready", 38, 600));
    status->AddChild(makeText(
        L"NeoWPF is running its shared application and rendering loop.", 24, 400));

    auto statusCard = std::make_shared<Border>();
    statusCard->SetPadding(32);
    statusCard->SetBorderThickness(1);
    statusCard->SetCornerRadius(NeoWPF::UI::CornerRadius(8));
    statusCard->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Surface));
    statusCard->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    statusCard->SetChild(status);
    content->AddChild(statusCard);

    content->AddChild(makeText(L"Android | arm64-v8a", 22, 400));

    auto root = std::make_shared<Grid>();
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);
    root->AddChild(content, 0, 0);

    windowHost->SetContent(root);
    auto window = NeoWPF::Core::Application::Current().CreateWindow(windowHost, 800, 480);

    const int result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
