#include <gtest/gtest.h>

#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

#include <memory>

namespace {

class ScrollContentProbe final : public NeoWPF::UI::FrameworkElement {
public:
    ScrollContentProbe(double width, double height)
        : m_width(width)
        , m_height(height)
    {
        SetWidth(width);
        SetHeight(height);
    }

    void Measure(double, double) override
    {
        SetDesiredSize(m_width, m_height);
    }

private:
    double m_width;
    double m_height;
};

TEST(ScrollViewerTests, NestedWheelInputIsConsumedAndChainsAtInnerBoundary)
{
    auto outer = std::make_shared<NeoWPF::UI::ScrollViewer>();
    auto inner = std::make_shared<NeoWPF::UI::ScrollViewer>();
    outer->SetVerticalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Hidden);
    inner->SetVerticalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Hidden);
    inner->SetHeight(120.0);
    inner->SetContent(std::make_shared<ScrollContentProbe>(80.0, 320.0));
    outer->SetContent(inner);
    outer->Measure(80.0, 40.0);
    outer->Arrange(80.0, 40.0);

    const auto raiseWheel = [&inner](double wheelDelta) {
        const NeoWPF::Input::Core::PointerEventArgs packet(
            1, NeoWPF::Input::Core::PointerDeviceType::Mouse,
            NeoWPF::Input::Core::PointerPhase::Wheel,
            10.0, 10.0, 0, 0, 1.0, wheelDelta);
        NeoWPF::Input::Core::RoutedPointerEventArgs args(
            NeoWPF::Input::Core::PointerWheelEvent(), packet);
        args.SetSource(inner.get());
        args.SetOriginalSource(inner.get());
        inner->RaiseEvent(&args);
        return args.IsHandled();
    };

    EXPECT_TRUE(raiseWheel(-120.0));
    ASSERT_GT(inner->GetVerticalOffset(), 0.0);
    EXPECT_DOUBLE_EQ(outer->GetVerticalOffset(), 0.0);

    inner->ScrollToBottom();
    const double innerBottom = inner->GetVerticalOffset();
    ASSERT_GT(innerBottom, 0.0);
    EXPECT_TRUE(raiseWheel(-120.0));
    EXPECT_DOUBLE_EQ(inner->GetVerticalOffset(), innerBottom);
    EXPECT_GT(outer->GetVerticalOffset(), 0.0);
}

} // namespace