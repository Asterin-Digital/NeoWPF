#include <NeoWPF/NeoWPF.h>

#include <memory>
#include <string>

int main()
{
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Trace);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::GridLayout,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::ContentPresenter,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Control,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::HitTest,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Virtualization,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Image,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::TextBox,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Http,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::XamlParser,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Popup,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::ItemsControl,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Clipboard,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Visual,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Diagnostics::SetLevel(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Off);
    NeoWPF::Core::Application::Initialize();

    auto windowHost = std::make_shared<NeoWPF::UI::Window>();
    windowHost->SetTitle(L"NeoWPF Android");

    using NeoWPF::UI::Button;
    using NeoWPF::UI::Border;
    using NeoWPF::UI::Checkbox;
    using NeoWPF::UI::HorizontalAlignment;
    using NeoWPF::UI::Orientation;
    using NeoWPF::UI::ScrollBarVisibility;
    using NeoWPF::UI::ScrollViewer;
    using NeoWPF::UI::StackPanel;
    using NeoWPF::UI::TextBlock;
    using NeoWPF::UI::TextBox;
    using NeoWPF::UI::Slider;

    const auto makeText = [](const std::wstring& value, int fontSize, int fontWeight) {
        auto text = std::make_shared<TextBlock>(value);
        text->SetFontSize(fontSize);
        text->SetFontWeight(fontWeight);
        return text;
    };

    auto content = std::make_shared<StackPanel>();
    content->SetSpacing(40);

    content->AddChild(makeText(L"Android controls", 72, 650));

    auto inputLabel = makeText(L"Text", 42, 600);
    content->AddChild(inputLabel);

    auto input = std::make_shared<TextBox>();
    input->SetText(L"Android input");
    input->SetPlaceholderText(L"Enter text");
    input->SetFontSize(48);
    input->SetHeight(156);
    input->SetPadding(NeoWPF::UI::Thickness(28, 24, 28, 24));
    input->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    content->AddChild(input);

    content->AddChild(makeText(L"Preview", 42, 600));
    auto preview = makeText(L"", 48, 400);
    preview->SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    preview->SetBinding(TextBlock::TextProperty(),
        NeoWPF::UI::Data::Binding("Text").WithSource(input));

    auto previewSurface = std::make_shared<Border>();
    previewSurface->SetPadding(NeoWPF::UI::Thickness(28));
    previewSurface->SetBorderThickness(1);
    previewSurface->SetCornerRadius(NeoWPF::UI::CornerRadius(4));
    previewSurface->SetBorderBrush(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    previewSurface->SetBackgroundBrush(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Surface));
    previewSurface->SetChild(preview);
    content->AddChild(previewSurface);

    auto levelValue = makeText(L"Level: 35", 42, 600);
    content->AddChild(levelValue);

    auto slider = std::make_shared<Slider>();
    slider->SetMinimum(0);
    slider->SetMaximum(100);
    slider->SetValue(35);
    slider->SetHeight(132);
    slider->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    slider->ValueChanged += [levelValue](double value) {
        levelValue->SetText(L"Level: " + std::to_wstring(static_cast<int>(value)));
    };
    content->AddChild(slider);

    auto sliderAnimation = std::make_shared<NeoWPF::UI::Animation::DoubleAnimation>();
    sliderAnimation->SetFrom(0.0f);
    sliderAnimation->SetTo(100.0f);
    sliderAnimation->SetDuration(1.5f);
    sliderAnimation->SetAutoReverse(true);
    sliderAnimation->SetRepeatBehavior(NeoWPF::UI::Animation::RepeatBehavior::Loop);
    auto sliderStoryboard = std::make_shared<NeoWPF::UI::Animation::Storyboard>();
    sliderStoryboard->AddChild({}, "Value", sliderAnimation);
    NeoWPF::UI::BeginStoryboard(sliderStoryboard, slider.get());

    auto applyButton = std::make_shared<Button>(L"Apply");
    applyButton->SetFontSize(44);
    applyButton->SetHeight(144);
    applyButton->SetPadding(NeoWPF::UI::Thickness(32, 24, 32, 24));
    applyButton->SetHorizontalAlignment(HorizontalAlignment::Stretch);

    auto enableAction = std::make_shared<Checkbox>();
    enableAction->SetText(L"Enable action");
    enableAction->SetFontSize(40);
    enableAction->SetHeight(132);
    enableAction->SetPadding(NeoWPF::UI::Thickness(24, 20, 24, 20));
    enableAction->Checked += [applyButton]() { applyButton->SetEnabled(true); };
    enableAction->Unchecked += [applyButton]() { applyButton->SetEnabled(false); };
    enableAction->SetChecked(true);
    content->AddChild(enableAction);

    auto statusText = makeText(L"Ready", 40, 400);
    applyButton->Click += [input, statusText]() {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input,
            NeoWPF::Diagnostics::Level::Debug,
            "Android sample Apply button clicked");
        const auto& value = input->GetText();
        statusText->SetText(value.empty() ? L"Applied" : L"Applied: " + value);
    };
    content->AddChild(applyButton);

    auto statusSurface = std::make_shared<Border>();
    statusSurface->SetPadding(NeoWPF::UI::Thickness(28));
    statusSurface->SetBorderThickness(1);
    statusSurface->SetCornerRadius(NeoWPF::UI::CornerRadius(4));
    statusSurface->SetBorderBrush(
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
            NeoWPF::Core::ApplicationThemes::PaletteBrush::Border));
    statusSurface->SetChild(statusText);
    content->AddChild(statusSurface);

    auto page = std::make_shared<Border>();
    page->SetPadding(NeoWPF::UI::Thickness(64, 112, 64, 88));
    page->SetChild(content);

    auto root = std::make_shared<ScrollViewer>();
    root->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
    root->SetVerticalScrollBarVisibility(ScrollBarVisibility::Auto);
    root->SetContent(page);

    windowHost->SetContent(root);
    auto window = NeoWPF::Core::Application::Current().CreateWindow(windowHost, 800, 480);

    const int result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
