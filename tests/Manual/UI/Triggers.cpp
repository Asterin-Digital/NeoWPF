#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <memory>
#include <filesystem>

namespace {

class ScaleToAction : public NeoWPF::UI::Styling::Triggers::TriggerAction {
public:
    ScaleToAction(std::shared_ptr<NeoWPF::UI::Core::ScaleTransform> transform,
                  float targetScale,
                  float durationSeconds)
        : m_transform(std::move(transform)),
          m_targetScale(targetScale),
          m_durationSeconds(durationSeconds)
    {
    }

    void Invoke(NeoWPF::UI::UIElement* element) const override {
        if (!element || !m_transform) {
            return;
        }

        m_transform->AnimateTo(m_targetScale, m_durationSeconds);
    }

private:
    std::shared_ptr<NeoWPF::UI::Core::ScaleTransform> m_transform;
    float m_targetScale = 1.0f;
    float m_durationSeconds = 0.2f;
};

std::shared_ptr<NeoWPF::UI::Grid> CreateTriggerDemoWindow()
{
    using namespace NeoWPF::UI;
    using namespace NeoWPF::UI::Styling::Triggers;

    auto root = std::make_shared<Grid>();
    root->SetBackgroundColor(0xFF1E1E1Eu);

    auto mainPanel = std::make_shared<StackPanel>();
    mainPanel->SetMargin(40);
    mainPanel->SetSpacing(24);
    mainPanel->SetHorizontalAlignment(HorizontalAlignment::Center);
    mainPanel->SetVerticalAlignment(VerticalAlignment::Center);

    auto title = std::make_shared<TextBlock>(L"Trigger System Demo");
    title->SetFontSize(32);
    title->SetFontFamily(L"Impact");
    title->SetTextColor(0xFFFFFFFFu);
    title->SetHorizontalAlignment(HorizontalAlignment::Center);
    mainPanel->AddChild(title);

    auto section1Title = std::make_shared<TextBlock>(L"Demo 1: Property Trigger (IsEnabled state)");
    section1Title->SetFontSize(14);
    section1Title->SetTextColor(0xFFB0B0B0u);
    section1Title->SetFontStyle(FontStyle::Italic);
    mainPanel->AddChild(section1Title);

    auto enableCheckbox = std::make_shared<Checkbox>();
    enableCheckbox->SetText(L"Enable interactive button");
    enableCheckbox->SetChecked(true);
    enableCheckbox->SetTextColor(0xFFFFFFFFu);
    mainPanel->AddChild(enableCheckbox);

    auto interactiveButton = std::make_shared<Button>(L"Click me!");
    interactiveButton->SetWidth(200);
    interactiveButton->SetHeight(45);
    interactiveButton->SetBackgroundColor(0xFF4F86C6u);
    interactiveButton->SetHorizontalAlignment(HorizontalAlignment::Center);

    auto enableTrigger = std::make_shared<Trigger>("IsChecked", true);
    enableTrigger->AddSetter(Setter("IsEnabled", true));

    auto disableTrigger = std::make_shared<Trigger>("IsChecked", false);
    disableTrigger->AddSetter(Setter("IsEnabled", false));

    enableCheckbox->Checked += [interactiveButton, enableTrigger]() {
        enableTrigger->Apply(interactiveButton.get());
    };

    enableCheckbox->Unchecked += [interactiveButton, disableTrigger]() {
        disableTrigger->Apply(interactiveButton.get());
    };

    if (enableCheckbox->IsChecked()) {
        enableTrigger->Apply(interactiveButton.get());
    } else {
        disableTrigger->Apply(interactiveButton.get());
    }

    mainPanel->AddChild(interactiveButton);

    auto section2Title = std::make_shared<TextBlock>(L"Demo 2: Multiple Setters (Color triggers)");
    section2Title->SetFontSize(14);
    section2Title->SetTextColor(0xFFB0B0B0u);
    section2Title->SetFontStyle(FontStyle::Italic);
    mainPanel->AddChild(section2Title);

    auto colorButton = std::make_shared<Button>(L"Default");
    colorButton->SetWidth(200);
    colorButton->SetHeight(45);
    colorButton->SetBackgroundColor(0xFF4F86C6u);
    colorButton->SetHorizontalAlignment(HorizontalAlignment::Center);

    auto redTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Red"));
    redTrigger->AddSetter(Setter("BackgroundColor", 0xFFCC3333u));
    redTrigger->AddSetter(Setter("Text", std::wstring(L"Red")));

    auto greenTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Green"));
    greenTrigger->AddSetter(Setter("BackgroundColor", 0xFF33CC33u));
    greenTrigger->AddSetter(Setter("Text", std::wstring(L"Green")));

    auto blueTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Blue"));
    blueTrigger->AddSetter(Setter("BackgroundColor", 0xFF3333CCu));
    blueTrigger->AddSetter(Setter("Text", std::wstring(L"Blue")));

    auto defaultTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Default"));
    defaultTrigger->AddSetter(Setter("BackgroundColor", 0xFF4F86C6u));
    defaultTrigger->AddSetter(Setter("Text", std::wstring(L"Default")));

    colorButton->AddHandler(
        NeoWPF::Input::Core::MouseEnterEvent(),
        [colorButton, redTrigger, greenTrigger, blueTrigger, defaultTrigger](UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {
            auto currentText = colorButton->GetText();
            
            if (currentText == L"Default") {
                colorButton->SetText(std::wstring(L"Red"));
                redTrigger->Apply(colorButton.get());
            } else if (currentText == L"Red") {
                colorButton->SetText(std::wstring(L"Green"));
                greenTrigger->Apply(colorButton.get());
            } else if (currentText == L"Green") {
                colorButton->SetText(std::wstring(L"Blue"));
                blueTrigger->Apply(colorButton.get());
            } else {
                colorButton->SetText(std::wstring(L"Default"));
                defaultTrigger->Apply(colorButton.get());
            }
        });

    mainPanel->AddChild(colorButton);

    auto section3Title = std::make_shared<TextBlock>(L"Demo 3: EventTrigger (Simulated)");
    section3Title->SetFontSize(14);
    section3Title->SetTextColor(0xFFB0B0B0u);
    section3Title->SetFontStyle(FontStyle::Italic);
    mainPanel->AddChild(section3Title);

    auto eventTriggerButton = std::make_shared<Button>(L"Scale on hover");
    eventTriggerButton->SetWidth(200);
    eventTriggerButton->SetHeight(45);
    eventTriggerButton->SetBackgroundColor(0xFF8866CCu);
    eventTriggerButton->SetHorizontalAlignment(HorizontalAlignment::Center);

    auto scale = std::make_shared<ScaleTransform>(1.0f, 1.0f);
    eventTriggerButton->SetRenderTransform(scale);

    auto scaleInTrigger = std::make_shared<EventTrigger>("MouseEnter");
    scaleInTrigger->AddAction(std::make_shared<ScaleToAction>(scale, 1.15f, 0.2f));

    auto scaleOutTrigger = std::make_shared<EventTrigger>("MouseLeave");
    scaleOutTrigger->AddAction(std::make_shared<ScaleToAction>(scale, 1.0f, 0.2f));

    eventTriggerButton->AddEventTrigger(scaleInTrigger);
    eventTriggerButton->AddEventTrigger(scaleOutTrigger);

    mainPanel->AddChild(eventTriggerButton);

    auto section4Title = std::make_shared<TextBlock>(L"Demo 4: Trigger Collection");
    section4Title->SetFontSize(14);
    section4Title->SetTextColor(0xFFB0B0B0u);
    section4Title->SetFontStyle(FontStyle::Italic);
    mainPanel->AddChild(section4Title);

    auto collectionButton = std::make_shared<Button>(L"Cycle States");
    collectionButton->SetWidth(200);
    collectionButton->SetHeight(45);
    collectionButton->SetBackgroundColor(0xFFCC9933u);
    collectionButton->SetHorizontalAlignment(HorizontalAlignment::Center);

    auto triggerCollection = std::make_shared<TriggerCollection>();

    auto state1Trigger = std::make_shared<Trigger>("Text", std::wstring(L"State 1"));
    state1Trigger->AddSetter(Setter("BackgroundColor", 0xFFEE5533u));

    auto state2Trigger = std::make_shared<Trigger>("Text", std::wstring(L"State 2"));
    state2Trigger->AddSetter(Setter("BackgroundColor", 0xFF33CCEEu));

    auto state3Trigger = std::make_shared<Trigger>("Text", std::wstring(L"State 3"));
    state3Trigger->AddSetter(Setter("BackgroundColor", 0xFF33EE55u));

    triggerCollection->Add(state1Trigger);
    triggerCollection->Add(state2Trigger);
    triggerCollection->Add(state3Trigger);

    collectionButton->SetText(std::wstring(L"State 1"));
    auto currentState = std::make_shared<int>(1);

    collectionButton->AddHandler(
        NeoWPF::Input::Core::MouseEnterEvent(),
        [collectionButton, triggerCollection, state1Trigger, state2Trigger, state3Trigger, currentState](UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {
            *currentState = (*currentState % 3) + 1;
            
            if (*currentState == 1) {
                collectionButton->SetText(std::wstring(L"State 1"));
                state1Trigger->Apply(collectionButton.get());
            } else if (*currentState == 2) {
                collectionButton->SetText(std::wstring(L"State 2"));
                state2Trigger->Apply(collectionButton.get());
            } else {
                collectionButton->SetText(std::wstring(L"State 3"));
                state3Trigger->Apply(collectionButton.get());
            }
        });

    mainPanel->AddChild(collectionButton);

    auto statusBar = std::make_shared<TextBlock>(
        L"✓ Trigger system is working. Hover buttons to see triggers in action.");
    statusBar->SetFontSize(12);
    statusBar->SetTextColor(0xFF88DD88u);
    statusBar->SetHorizontalAlignment(HorizontalAlignment::Center);
    statusBar->SetMargin(20, 0, 20, 0);
    mainPanel->AddChild(statusBar);

    root->AddChild(mainPanel);
    return root;
}

}

int main()
{
    using namespace NeoWPF::Diagnostics;

    SetGlobalLevel(Level::Info);
    SetLevel(Category::GridLayout, Level::Off);
    SetLevel(Category::Application, Level::Trace);
    SetLevel(Category::HitTest, Level::Off);
    NeoWPF::Core::Application::Initialize();
    NeoWPF::Core::Application::SetResource("FontFamily", std::wstring(L"Impact"));

    auto& app = NeoWPF::Core::Application::Current();
    app.Resources().RegisterProvider(
        std::make_unique<NeoWPF::ExternalFileResourceProvider>(
            std::filesystem::current_path()));

    auto window = app.CreateWindow(L"NeoWPF Trigger Demo", 800, 900);
    window.SetContent(CreateTriggerDemoWindow());

    auto result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}
