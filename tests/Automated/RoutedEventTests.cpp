#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Layout/Canvas/Canvas.h>

#include <memory>
#include <vector>

namespace {

class RoutedEventTestChild final : public NeoWPF::UI::FrameworkElement {
public:
    bool IsInteractable() const noexcept override
    {
        return true;
    }

    void Measure(double, double) override
    {
        SetDesiredSize(40.0, 30.0);
    }
};

std::shared_ptr<RoutedEventTestChild> AddPointerTarget(NeoWPF::UI::Canvas& canvas)
{
    canvas.SetWidth(80.0);
    canvas.SetHeight(50.0);
    auto child = std::make_shared<RoutedEventTestChild>();
    canvas.AddChild(child);
    canvas.Measure(80.0, 50.0);
    canvas.Arrange(80.0, 50.0);
    return child;
}

void SubmitPointerDown(NeoWPF::Input::InputManager& input)
{
    const NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
        NeoWPF::Input::Core::PointerPhase::Down,
        10.0, 10.0, 0x01u, 0x01u);
    input.SubmitPointerEvent(packet);
}

TEST(RoutedEventTests, SourceRemainsRaisingElementDuringBubbling)
{
    NeoWPF::UI::Canvas canvas;
    auto child = std::make_shared<RoutedEventTestChild>();
    canvas.AddChild(child);

    static const NeoWPF::Input::Core::RoutedEvent routedEvent(
        "RoutedEventTests.Source", NeoWPF::Input::Core::RoutingStrategy::Bubble, 0xCA01);
    NeoWPF::UI::UIElement* observedSender = nullptr;
    NeoWPF::UI::UIElement* observedSource = nullptr;
    NeoWPF::UI::UIElement* observedOriginalSource = nullptr;
    canvas.AddHandler(routedEvent, [&](NeoWPF::UI::UIElement* sender,
                                      NeoWPF::Input::Core::RoutedEventArgs* args) {
        observedSender = sender;
        observedSource = args->GetSource();
        observedOriginalSource = args->GetOriginalSource();
    });

    NeoWPF::Input::Core::RoutedEventArgs args(routedEvent.Strategy);
    args.SetRoutedEvent(&routedEvent);
    child->RaiseEvent(&args);

    EXPECT_EQ(observedSender, &canvas);
    EXPECT_EQ(observedSource, child.get());
    EXPECT_EQ(observedOriginalSource, child.get());
}

TEST(RoutedEventTests, PointerBasesShareOneHandledState)
{
    NeoWPF::Input::Core::PointerEventArgs packet(
        1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
        NeoWPF::Input::Core::PointerPhase::Down, 0.0, 0.0);
    packet.SetHandled(true);
    NeoWPF::Input::Core::RoutedPointerEventArgs args(
        NeoWPF::Input::Core::PointerEvent(), packet);

    auto& packetArgs = static_cast<NeoWPF::Input::Core::PointerEventArgs&>(args);
    auto& routedArgs = static_cast<NeoWPF::Input::Core::RoutedEventArgs&>(args);
    EXPECT_TRUE(routedArgs.IsHandled());

    packetArgs.SetHandled(false);
    EXPECT_FALSE(routedArgs.IsHandled());
    routedArgs.SetHandled(true);
    EXPECT_TRUE(packetArgs.IsHandled());
}

TEST(RoutedEventTests, HandledPreviewKeyEventSuppressesPairedBubbleEvent)
{
    NeoWPF::UI::UIElement focusedElement;
    bool previewHandled = false;
    int bubbleHandlerCalls = 0;
    focusedElement.AddHandler(NeoWPF::Input::Core::PreviewKeyDownEvent(),
        [&previewHandled](NeoWPF::UI::UIElement*,
                          NeoWPF::Input::Core::RoutedEventArgs* args) {
            previewHandled = true;
            args->SetHandled(true);
        });
    focusedElement.AddHandler(NeoWPF::Input::Core::KeyDownEvent(),
        [&bubbleHandlerCalls](NeoWPF::UI::UIElement*,
                              NeoWPF::Input::Core::RoutedEventArgs*) {
            ++bubbleHandlerCalls;
        });

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.SetFocusedElement(&focusedElement);
    input.ProcessKeyboardInput(0x41, true);
    input.SetFocusedElement(nullptr);
    input.ProcessKeyboardInput(0x41, false);

    EXPECT_TRUE(previewHandled);
    EXPECT_EQ(bubbleHandlerCalls, 0);
}

TEST(RoutedEventTests, PointerDownTunnelsPreviewBeforeBubblingPress)
{
    NeoWPF::UI::Canvas canvas;
    const auto child = AddPointerTarget(canvas);
    std::vector<NeoWPF::UI::UIElement*> routedElements;

    canvas.AddHandler(NeoWPF::Input::Core::PreviewPointerPressedEvent(),
        [&routedElements](NeoWPF::UI::UIElement* sender,
                          NeoWPF::Input::Core::RoutedEventArgs*) {
            routedElements.push_back(sender);
        });
    child->AddHandler(NeoWPF::Input::Core::PreviewPointerPressedEvent(),
        [&routedElements](NeoWPF::UI::UIElement* sender,
                          NeoWPF::Input::Core::RoutedEventArgs*) {
            routedElements.push_back(sender);
        });
    child->AddHandler(NeoWPF::Input::Core::PointerPressedEvent(),
        [&routedElements](NeoWPF::UI::UIElement* sender,
                          NeoWPF::Input::Core::RoutedEventArgs*) {
            routedElements.push_back(sender);
        });
    canvas.AddHandler(NeoWPF::Input::Core::PointerPressedEvent(),
        [&routedElements](NeoWPF::UI::UIElement* sender,
                          NeoWPF::Input::Core::RoutedEventArgs*) {
            routedElements.push_back(sender);
        });

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.NotifyVisualTreeReplaced();
    input.SetRootElement(&canvas);
    EXPECT_EQ(input.HitTest(10, 10), child.get());
    SubmitPointerDown(input);

    EXPECT_EQ(routedElements, (std::vector<NeoWPF::UI::UIElement*>{
        &canvas, child.get(), child.get(), &canvas}));
    input.NotifyVisualTreeReplaced();
}

TEST(RoutedEventTests, HandledPointerPreviewSuppressesPointerPressed)
{
    NeoWPF::UI::Canvas canvas;
    const auto child = AddPointerTarget(canvas);
    bool previewHandled = false;
    int bubbleHandlerCalls = 0;
    canvas.AddHandler(NeoWPF::Input::Core::PreviewPointerPressedEvent(),
        [&previewHandled](NeoWPF::UI::UIElement*,
                          NeoWPF::Input::Core::RoutedEventArgs* args) {
            previewHandled = true;
            args->SetHandled(true);
        });
    child->AddHandler(NeoWPF::Input::Core::PointerPressedEvent(),
        [&bubbleHandlerCalls](NeoWPF::UI::UIElement*,
                              NeoWPF::Input::Core::RoutedEventArgs*) {
            ++bubbleHandlerCalls;
        });

    auto& input = NeoWPF::Input::InputManager::GetInstance();
    input.NotifyVisualTreeReplaced();
    input.SetRootElement(&canvas);
    EXPECT_EQ(input.HitTest(10, 10), child.get());
    SubmitPointerDown(input);

    EXPECT_TRUE(previewHandled);
    EXPECT_EQ(bubbleHandlerCalls, 0);
    input.NotifyVisualTreeReplaced();
}

TEST(RoutedEventTests, KeyboardBubbleKeepsPreviewTargetWhenFocusChanges)
{
    NeoWPF::UI::UIElement previewTarget;
    NeoWPF::UI::UIElement newFocusTarget;
    int previewTargetBubbleCalls = 0;
    int newFocusBubbleCalls = 0;
    auto& input = NeoWPF::Input::InputManager::GetInstance();

    previewTarget.AddHandler(NeoWPF::Input::Core::PreviewKeyDownEvent(),
        [&input, &newFocusTarget](NeoWPF::UI::UIElement*,
                                 NeoWPF::Input::Core::RoutedEventArgs*) {
            input.SetFocusedElement(&newFocusTarget);
        });
    previewTarget.AddHandler(NeoWPF::Input::Core::KeyDownEvent(),
        [&previewTargetBubbleCalls](NeoWPF::UI::UIElement*,
                                   NeoWPF::Input::Core::RoutedEventArgs*) {
            ++previewTargetBubbleCalls;
        });
    newFocusTarget.AddHandler(NeoWPF::Input::Core::KeyDownEvent(),
        [&newFocusBubbleCalls](NeoWPF::UI::UIElement*,
                              NeoWPF::Input::Core::RoutedEventArgs*) {
            ++newFocusBubbleCalls;
        });

    input.SetFocusedElement(&previewTarget);
    input.ProcessKeyboardInput(0x42, true);
    input.SetFocusedElement(nullptr);
    input.ProcessKeyboardInput(0x42, false);

    EXPECT_EQ(previewTargetBubbleCalls, 1);
    EXPECT_EQ(newFocusBubbleCalls, 0);
}

}