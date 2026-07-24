#include <memory>
#include <string>
#include <NeoWPF/NeoWPF.h>

int main()
{
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Core::Application::Current()->CreateWindow(L"NeoApp", 900, 680);

    using namespace NeoWPF;
    using namespace NeoWPF::UI;

    auto root = std::make_shared<StackPanel>();
    root->SetBackgroundColor(0xFF0F172Au);

    auto title = std::make_shared<TextBlock>(L"NeoWPF scrollbar demo");
    title->SetFontSize(28);
    title->SetTextColor(0xFFEAEEF8u);

    auto subtitle = std::make_shared<TextBlock>(L"6,000 text blocks in a scrollable panel");
    subtitle->SetFontSize(16);
    subtitle->SetTextColor(0xFF94A3B8u);

    auto viewport = std::make_shared<Border>();
    viewport->SetBorderThickness(1);
    viewport->SetBorderColor(0xFF334155u);
    viewport->SetBackgroundColor(0xFF111827u);
    viewport->SetCornerRadius(CornerRadius(16.0f));
    viewport->SetPadding(8);
    viewport->SetClipToBounds(true);
    viewport->SetWidth(720);
    viewport->SetHeight(520);

    auto contentHost = std::make_shared<StackPanel>();
    contentHost->SetBackgroundColor(0x00000000u);

    for (int i = 1; i <= 6000; ++i) {
        auto line = std::make_shared<TextBlock>(L"Item " + std::to_wstring(i));
        line->SetFontSize(14);
        line->SetTextColor(i % 2 == 0 ? 0xFFE2E8F0u : 0xFFCBD5E1u);
        contentHost->AddChild(line);
    }

    viewport->SetChild(contentHost);

    auto layout = std::make_shared<Grid>();
    layout->AddRow(SizeMode::Auto, 0);
    layout->AddColumn(SizeMode::Star, 1);
    layout->AddColumn(SizeMode::Auto, 0);
    layout->AddChild(viewport, 0, 0);

    auto scrollBar = std::make_shared<ScrollBar>();
    scrollBar->SetOrientation(Orientation::Vertical);
    scrollBar->SetScrollTarget(contentHost);
    scrollBar->SetViewportElement(viewport);
    scrollBar->SetTrackColor(0xFF1E293Bu);
    scrollBar->SetThumbColor(0xFF5B6B7Au);
    scrollBar->SetThumbSize(24);
    layout->AddChild(scrollBar, 0, 1);

    root->AddChild(title);
    root->AddChild(subtitle);
    root->AddChild(layout);

    return NeoWPF::Core::Application::Run();
}

///////////////////////////////////////////////////////////////////////////////
// Result: Pass 
// (No crash or unresponsive state, app opened though with lag when scrolling.)
// [Virtualization Not Present During This Test])
// Tested on: V1.0.0 (Dev)
///////////////////////////////////////////////////////////////////////////////