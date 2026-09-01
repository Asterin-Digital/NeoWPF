#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <memory>

namespace {

class ScaleToAction : public NeoWPF::UI::Triggers::TriggerAction {
public:
    ScaleToAction(std::shared_ptr<NeoWPF::UI::Core::ScaleTransform> transform,
                  float targetScale,
                  float durationSeconds)
        : m_transform(std::move(transform))
        , m_targetScale(targetScale)
        , m_durationSeconds(durationSeconds)
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

class DarkRoundedButtonTemplate : public NeoWPF::UI::Templates::ControlTemplate {
public:
    DarkRoundedButtonTemplate()
    {
        auto root = std::make_shared<NeoWPF::UI::Visual>();
        SetVisualTree(root);

        auto background = std::make_shared<NeoWPF::UI::Templates::TemplateVisual>("PART_Background");
        background->SetFillColor(0xFF1A1A1Au);
        background->SetBorderColor(0xFF4B4B4Bu);
        background->SetBorderThickness(1);
        background->SetCornerRadius(12.0f);

        auto contentHost = std::make_shared<NeoWPF::UI::Templates::TemplateVisual>("PART_ContentHost");
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);

        AddTemplateChild("PART_Background", background);
        AddTemplateChild("PART_ContentHost", contentHost);

        auto stateManager = std::make_shared<NeoWPF::UI::Styling::VisualStateManager>();
        auto commonStates = std::make_shared<NeoWPF::UI::Styling::VisualStateGroup>("CommonStates");

        auto normal = std::make_shared<NeoWPF::UI::Styling::VisualState>("Normal");
        normal->AddSetter("PART_Background", [](NeoWPF::UI::UIElement* element, NeoWPF::UI::Visual* visual) {
            if (auto templateVisual = dynamic_cast<NeoWPF::UI::Templates::TemplateVisual*>(visual)) {
                templateVisual->SetFillColor(0xFF1A1A1Au);
                templateVisual->SetBorderColor(0xFF4B4B4Bu);
            }
            if (element) {
                element->InvalidateVisual();
            }
        });
        commonStates->AddState(normal);

        auto mouseOver = std::make_shared<NeoWPF::UI::Styling::VisualState>("MouseOver");
        mouseOver->AddSetter("PART_Background", [](NeoWPF::UI::UIElement* element, NeoWPF::UI::Visual* visual) {
            if (auto templateVisual = dynamic_cast<NeoWPF::UI::Templates::TemplateVisual*>(visual)) {
                templateVisual->SetFillColor(0xFF2A2A2Au);
                templateVisual->SetBorderColor(0xFF6AA6FFu);
            }
            if (element) {
                element->InvalidateVisual();
            }
        });
        commonStates->AddState(mouseOver);

        auto pressed = std::make_shared<NeoWPF::UI::Styling::VisualState>("Pressed");
        pressed->AddSetter("PART_Background", [](NeoWPF::UI::UIElement* element, NeoWPF::UI::Visual* visual) {
            if (auto templateVisual = dynamic_cast<NeoWPF::UI::Templates::TemplateVisual*>(visual)) {
                templateVisual->SetFillColor(0xFF111111u);
                templateVisual->SetBorderColor(0xFF8BB4FFu);
            }
            if (element) {
                element->InvalidateVisual();
            }
        });
        commonStates->AddState(pressed);

        auto disabled = std::make_shared<NeoWPF::UI::Styling::VisualState>("Disabled");
        disabled->AddSetter("PART_Background", [](NeoWPF::UI::UIElement* element, NeoWPF::UI::Visual* visual) {
            if (auto templateVisual = dynamic_cast<NeoWPF::UI::Templates::TemplateVisual*>(visual)) {
                templateVisual->SetFillColor(0xFF2F2F2Fu);
                templateVisual->SetBorderColor(0xFF555555u);
            }
            if (element) {
                element->InvalidateVisual();
            }
        });
        commonStates->AddState(disabled);

        stateManager->AddGroup(commonStates);
        SetVisualStateManager(stateManager);
    }
};

std::shared_ptr<NeoWPF::UI::Button> CreateTestButton()
{
    using namespace NeoWPF::UI;
    using namespace NeoWPF::UI::Triggers;

    auto button = std::make_shared<Button>(L"Test");
    button->SetTemplate(std::make_shared<DarkRoundedButtonTemplate>());
    button->SetWidth(180);
    button->SetHeight(52);
    button->SetCornerRadius(12.0f);
    button->SetBackgroundColor(0xFF1A1A1Au);
    button->SetBorderColor(0xFF4B4B4Bu);
    button->SetTextColor(0xFFEAEAEAu);
    button->SetPadding(12);

    auto scale = std::make_shared<NeoWPF::UI::Core::ScaleTransform>(1.0f, 1.0f);
    button->SetRenderTransform(scale);

    auto hoverIn = std::make_shared<EventTrigger>("MouseEnter");
    hoverIn->AddAction(std::make_shared<ScaleToAction>(scale, 1.08f, 0.18f));

    auto hoverOut = std::make_shared<EventTrigger>("MouseLeave");
    hoverOut->AddAction(std::make_shared<ScaleToAction>(scale, 1.0f, 0.18f));

    button->AddEventTrigger(hoverIn);
    button->AddEventTrigger(hoverOut);

    auto idleTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Idle"));
    idleTrigger->AddSetter(Setter("BackgroundColor", 0xFF1A1A1Au));
    idleTrigger->AddSetter(Setter("BorderColor", 0xFF4B4B4Bu));
    idleTrigger->AddSetter(Setter("Text", std::wstring(L"Idle")));

    auto readyTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Ready"));
    readyTrigger->AddSetter(Setter("BackgroundColor", 0xFF2A4D8Fu));
    readyTrigger->AddSetter(Setter("BorderColor", 0xFF73A4FFu));
    readyTrigger->AddSetter(Setter("Text", std::wstring(L"Ready")));

    auto dangerTrigger = std::make_shared<Trigger>("Text", std::wstring(L"Danger"));
    dangerTrigger->AddSetter(Setter("BackgroundColor", 0xFF7A2E2Eu));
    dangerTrigger->AddSetter(Setter("BorderColor", 0xFFFF7F7Fu));
    dangerTrigger->AddSetter(Setter("Text", std::wstring(L"Danger")));

    auto states = std::make_shared<TriggerCollection>();
    states->Add(idleTrigger);
    states->Add(readyTrigger);
    states->Add(dangerTrigger);

    button->Click += [button, idleTrigger, readyTrigger, dangerTrigger, states]() {
        auto current = button->GetText();

        if (current == L"Idle") {
            button->SetText(std::wstring(L"Ready"));
            readyTrigger->Apply(button.get());
        } else if (current == L"Ready") {
            button->SetText(std::wstring(L"Danger"));
            dangerTrigger->Apply(button.get());
        } else {
            button->SetText(std::wstring(L"Idle"));
            idleTrigger->Apply(button.get());
        }
    };

    button->SetText(std::wstring(L"Idle"));
    idleTrigger->Apply(button.get());

    return button;
}

std::shared_ptr<NeoWPF::UI::Grid> CreateWindowContent()
{
    using namespace NeoWPF::UI;

    auto root = std::make_shared<Grid>();
    root->SetBackgroundColor(0xFF121212u);

    auto button = CreateTestButton();
    button->SetHorizontalAlignment(HorizontalAlignment::Center);
    button->SetVerticalAlignment(VerticalAlignment::Center);

    root->AddChild(button);
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
            NeoWPF::Platform::CurrentPlatform()->GetExecutableDirectory()));

    auto window = app.CreateWindow(L"NeoWPF Trigger Test", 420, 220);
    window.SetContent(CreateWindowContent());

    return NeoWPF::Core::Application::Run();
}
