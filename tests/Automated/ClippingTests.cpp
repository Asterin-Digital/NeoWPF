#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <any>
#include <memory>
#include <unordered_set>
#include <vector>

namespace {

class ClipProbe final : public NeoWPF::UI::FrameworkElement {
public:
    ClipProbe(int marker, double width, double height)
        : m_marker(marker)
        , m_width(width)
        , m_height(height)
    {
        SetWidth(width);
        SetHeight(height);
        SetClipToBounds(true);
    }

    void Measure(double, double) override
    {
        SetDesiredSize(m_width, m_height);
    }

    int GetMarker() const noexcept
    {
        return m_marker;
    }

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override
    {
        commands.DrawRect(GetLayoutOffsetX(),
                          GetLayoutOffsetY(),
                          GetFinalWidth(),
                          GetFinalHeight(),
                          NeoWPF::Rendering::Paint::Solid(0xFF123456u),
                          1.0f,
                          GetClipRect());
    }

private:
    int m_marker;
    double m_width;
    double m_height;
};

void ExpectClip(const NeoWPF::Rendering::ClipRect& clip,
                double x,
                double y,
                double width,
                double height)
{
    ASSERT_TRUE(clip.active);
    EXPECT_DOUBLE_EQ(clip.x, x);
    EXPECT_DOUBLE_EQ(clip.y, y);
    EXPECT_DOUBLE_EQ(clip.w, width);
    EXPECT_DOUBLE_EQ(clip.h, height);
}

void ExpectClipWithin(const NeoWPF::Rendering::ClipRect& clip,
                      const NeoWPF::Rendering::ClipRect& viewportClip)
{
    ASSERT_TRUE(clip.active);
    if (clip.w <= 0 || clip.h <= 0) {
        return;
    }

    EXPECT_GE(clip.x, viewportClip.x);
    EXPECT_GE(clip.y, viewportClip.y);
    EXPECT_LE(clip.x + clip.w, viewportClip.x + viewportClip.w);
    EXPECT_LE(clip.y + clip.h, viewportClip.y + viewportClip.h);
}

std::shared_ptr<ClipProbe> FindItem(const NeoWPF::UI::ItemsControl& itemsControl,
                                    int marker)
{
    for (auto* child : itemsControl.GetVisualChildren()) {
        auto* probe = dynamic_cast<ClipProbe*>(child);
        if (probe && probe->GetMarker() == marker) {
            return std::dynamic_pointer_cast<ClipProbe>(probe->shared_from_this());
        }
    }
    return nullptr;
}

std::vector<std::any> MakeItems(int count)
{
    std::vector<std::any> items;
    items.reserve(static_cast<std::size_t>(count));
    for (int index = 0; index < count; ++index) {
        items.emplace_back(index);
    }
    return items;
}

TEST(FrameworkElementClippingTests, DisabledClipDoesNotRestrictElement)
{
    ClipProbe element(0, 80, 50);
    element.SetClipToBounds(false);
    element.Arrange(80, 50);

    EXPECT_FALSE(element.GetClipRect().active);
}

TEST(FrameworkElementClippingTests, OwnClipUsesArrangedBoundsAndLayoutOffset)
{
    ClipProbe element(0, 80, 50);
    element.Arrange(80, 50);
    element.SetLayoutOffset(12, 24);

    ExpectClip(element.GetClipRect(), 12, 24, 80, 50);
}

TEST(FrameworkElementClippingTests, UnclippedChildInheritsNearestAncestorClip)
{
    auto parent = std::make_shared<NeoWPF::UI::StackPanel>();
    auto child = std::make_shared<ClipProbe>(0, 30, 20);
    parent->SetClipToBounds(true);
    parent->AddChild(child);
    parent->Arrange(100, 80);
    parent->SetLayoutOffset(10, 20);
    child->SetClipToBounds(false);
    child->Arrange(30, 20);
    child->SetLayoutOffset(200, 160);

    ExpectClip(parent->GetClipRect(), 10, 20, 100, 80);
    ExpectClip(child->GetClipRect(), 10, 20, 100, 80);
}

TEST(FrameworkElementClippingTests, NestedClipsUseTheirVisibleIntersection)
{
    auto parent = std::make_shared<NeoWPF::UI::StackPanel>();
    auto child = std::make_shared<ClipProbe>(0, 50, 50);
    parent->SetClipToBounds(true);
    parent->AddChild(child);
    parent->Arrange(100, 80);
    parent->SetLayoutOffset(10, 20);
    child->Arrange(50, 50);
    child->SetLayoutOffset(80, 70);

    ExpectClip(child->GetClipRect(), 90, 90, 20, 10);
}

TEST(FrameworkElementClippingTests, EdgeTouchingClipProducesEmptyActiveIntersection)
{
    auto parent = std::make_shared<NeoWPF::UI::StackPanel>();
    auto child = std::make_shared<ClipProbe>(0, 40, 20);
    parent->SetClipToBounds(true);
    parent->AddChild(child);
    parent->Arrange(100, 80);
    child->Arrange(40, 20);
    child->SetLayoutOffset(100, 10);

    ExpectClip(child->GetClipRect(), 100, 10, 0, 0);
}

TEST(ScrollViewerClippingTests, ScrolledContentAndDrawCommandsStayInsideViewport)
{
    NeoWPF::UI::ScrollViewer viewer;
    viewer.SetHorizontalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Disabled);
    viewer.SetVerticalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Disabled);
    auto content = std::make_shared<ClipProbe>(0, 160, 120);
    viewer.SetContent(content);
    viewer.Measure(80, 40);
    viewer.Arrange(80, 40);

    ExpectClip(viewer.GetClipRect(), 0, 0, 80, 40);
    ExpectClip(content->GetClipRect(), 0, 0, 80, 40);

    viewer.SetVerticalOffset(30);

    ExpectClip(content->GetClipRect(), 0, 0, 80, 40);
    NeoWPF::Rendering::CommandList commands;
    viewer.BuildRenderCommands(commands);
    ASSERT_EQ(commands.commands.size(), 1u);
    ASSERT_EQ(commands.commands[0].GetKind(),
              NeoWPF::Rendering::CommandList::CommandKind::Rect);
    ExpectClip(commands.commands[0].GetRect().clip, 0, 0, 80, 40);
}

TEST(ItemsControlClippingTests, VirtualizedPartialRowClipsHorizontallyAndVertically)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource(MakeItems(20));
    itemsControl.SetItemTemplate([](const std::any&, int index) {
        return std::make_shared<ClipProbe>(index, 90, 10);
    });
    itemsControl.SetVirtualization(true);
    itemsControl.Measure(60, 25);
    itemsControl.Arrange(60, 25);

    ASSERT_EQ(itemsControl.GetFinalWidth(), 60);
    ASSERT_EQ(itemsControl.GetFinalHeight(), 25);
    ExpectClip(itemsControl.GetClipRect(), 0, 0, 60, 25);

    const auto partiallyVisible = FindItem(itemsControl, 2);
    ASSERT_NE(partiallyVisible, nullptr);
    EXPECT_EQ(partiallyVisible->GetLayoutOffsetY(), 20);
    EXPECT_EQ(partiallyVisible->GetFinalWidth(), 90);
    ExpectClip(partiallyVisible->GetClipRect(), 0, 20, 60, 5);

    NeoWPF::Rendering::CommandList commands;
    partiallyVisible->BuildRenderCommands(commands);
    ASSERT_EQ(commands.commands.size(), 1u);
    ExpectClip(commands.commands[0].GetRect().clip, 0, 20, 60, 5);
}

TEST(ItemsControlClippingTests, BufferedRowsOutsideScrolledViewportHaveEmptyClips)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource(MakeItems(30));
    itemsControl.SetItemTemplate([](const std::any&, int index) {
        return std::make_shared<ClipProbe>(index, 90, 10);
    });
    itemsControl.SetVirtualization(true);
    itemsControl.Measure(60, 25);
    itemsControl.Arrange(60, 25);

    itemsControl.SetVerticalOffset(45);
    itemsControl.Measure(60, 25);
    itemsControl.Arrange(60, 25);

    int fullyOutsideCount = 0;
    int visibleCount = 0;
    const auto clip = itemsControl.GetClipRect();
    ExpectClip(clip, 0, 0, 60, 25);
    for (auto* child : itemsControl.GetVisualChildren()) {
        const auto item = dynamic_cast<ClipProbe*>(child);
        ASSERT_NE(item, nullptr);
        const double top = item->GetLayoutOffsetY();
        const double bottom = top + item->GetFinalHeight();
        const auto itemClip = item->GetClipRect();
        ASSERT_TRUE(itemClip.active);

        if (bottom <= 0) {
            ++fullyOutsideCount;
            EXPECT_DOUBLE_EQ(itemClip.w, 0);
            EXPECT_DOUBLE_EQ(itemClip.h, 0);
        } else if (top < 25 && bottom > 0) {
            ++visibleCount;
            EXPECT_GE(itemClip.x, clip.x);
            EXPECT_GE(itemClip.y, clip.y);
            EXPECT_LE(itemClip.x + itemClip.w, clip.x + clip.w);
            EXPECT_LE(itemClip.y + itemClip.h, clip.y + clip.h);
        }
    }

    EXPECT_GT(fullyOutsideCount, 0);
    EXPECT_GT(visibleCount, 0);
}

TEST(ItemsControlClippingTests, TenThousandItemsKeepRealizedClipsBoundedAcrossScrollPositions)
{
    constexpr int itemCount = 10000;
    constexpr double viewportWidth = 64;
    constexpr double viewportHeight = 23;

    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource(MakeItems(itemCount));
    itemsControl.SetItemTemplate([](const std::any&, int index) {
        return std::make_shared<ClipProbe>(index, 96, 8);
    });
    itemsControl.SetVirtualization(true);

    const std::vector<double> offsets{0, 40000, 77777, 200000};
    for (const double requestedOffset : offsets) {
        itemsControl.SetVerticalOffset(requestedOffset);
        itemsControl.Measure(viewportWidth, viewportHeight);
        itemsControl.Arrange(viewportWidth, viewportHeight);

        const auto virtualizer = itemsControl.GetVirtualization();
        ASSERT_NE(virtualizer, nullptr);
        const int firstRealized = virtualizer->GetFirstRealizedIndex();
        const int lastRealized = virtualizer->GetLastRealizedIndex();
        ASSERT_GE(firstRealized, 0);
        ASSERT_LT(lastRealized, itemCount);
        EXPECT_LE(lastRealized - firstRealized + 1, 32);
        EXPECT_LT(itemsControl.GetVisualChildren().size(), static_cast<std::size_t>(itemCount));
        EXPECT_GE(virtualizer->GetVerticalOffset(), 0);
        EXPECT_LE(virtualizer->GetVerticalOffset(),
                  (std::max)(0.0, virtualizer->GetExtentHeight() - viewportHeight));

        const auto viewportClip = itemsControl.GetClipRect();
        ExpectClip(viewportClip, 0, 0, viewportWidth, viewportHeight);

        std::unordered_set<int> realizedIndices;
        std::size_t visibleCount = 0;
        std::size_t emptyClipCount = 0;
        NeoWPF::Rendering::CommandList commands;
        itemsControl.BuildRenderCommands(commands);
        ASSERT_EQ(commands.commands.size(), itemsControl.GetVisualChildren().size());

        for (std::size_t childIndex = 0;
             childIndex < itemsControl.GetVisualChildren().size();
             ++childIndex) {
            const auto item = dynamic_cast<ClipProbe*>(itemsControl.GetVisualChildren()[childIndex]);
            ASSERT_NE(item, nullptr);
            SCOPED_TRACE(::testing::Message()
                         << "requestedOffset=" << requestedOffset
                         << " firstRealized=" << firstRealized
                         << " lastRealized=" << lastRealized
                         << " marker=" << item->GetMarker()
                         << " top=" << item->GetLayoutOffsetY()
                         << " parent=" << item->GetParent()
                         << " expectedParent=" << &itemsControl);
            ASSERT_EQ(item->GetParent(), &itemsControl);
            const auto dataContext = item->GetDataContext();
            ASSERT_TRUE(dataContext.has_value());
            ASSERT_EQ(dataContext.type(), typeid(int));
            const int itemIndex = std::any_cast<int>(dataContext);
            EXPECT_GE(itemIndex, 0);
            EXPECT_LT(itemIndex, itemCount);
            EXPECT_TRUE(realizedIndices.insert(itemIndex).second);

            const auto clip = item->GetClipRect();
            SCOPED_TRACE(::testing::Message()
                         << "requestedOffset=" << requestedOffset
                         << " realizedRange=" << firstRealized << ".." << lastRealized
                         << " item=" << itemIndex
                         << " clip=(" << clip.x << ',' << clip.y << ','
                         << clip.w << ',' << clip.h << ')');
            ExpectClipWithin(clip, viewportClip);
            if (clip.w > 0 && clip.h > 0) {
                ++visibleCount;
            }
            if (clip.w == 0 || clip.h == 0) {
                ++emptyClipCount;
            }

            ASSERT_EQ(commands.commands[childIndex].GetKind(),
                      NeoWPF::Rendering::CommandList::CommandKind::Rect);
            const auto& commandClip = commands.commands[childIndex].GetRect().clip;
            ExpectClipWithin(commandClip, viewportClip);
        }

        EXPECT_GT(visibleCount, 0u);
        EXPECT_GT(emptyClipCount, 0u);
    }
}

}