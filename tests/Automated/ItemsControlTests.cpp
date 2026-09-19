#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemContainerGenerator.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

#include <any>
#include <memory>
#include <vector>

namespace {

class FixedItem final : public NeoWPF::UI::FrameworkElement {
public:
    FixedItem(int width, int height, int marker = 0)
        : m_width(width)
        , m_height(height)
        , m_marker(marker)
    {
    }

    void Measure(double, double) override
    {
        SetDesiredSize(m_width, m_height);
    }

    int GetMarker() const noexcept
    {
        return m_marker;
    }

private:
    int m_width;
    int m_height;
    int m_marker;
};

TEST(ItemsControlTests, InlineItemsMeasureAndArrangeWithSpacingAndPadding)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetHeightMode(NeoWPF::UI::LayoutSizeMode::Auto);
    itemsControl.SetPadding(2);
    itemsControl.SetSpacing(5);

    auto first = std::make_shared<FixedItem>(20, 10, 1);
    auto second = std::make_shared<FixedItem>(30, 15, 2);
    itemsControl.AddItem(first);
    itemsControl.AddItem(second);

    itemsControl.Measure(100, 100);
    itemsControl.Arrange(100, 100);

    EXPECT_EQ(itemsControl.GetItemCount(), 2);
    EXPECT_EQ(first->GetLayoutOffsetY(), 2);
    EXPECT_EQ(second->GetLayoutOffsetY(), 17);
    EXPECT_EQ(first->GetFinalHeight(), 10);
    EXPECT_EQ(second->GetFinalHeight(), 15);
}

TEST(ItemsControlTests, EmptyItemsSourceReplacesInlineItems)
{
    NeoWPF::UI::ItemsControl itemsControl;
    auto inlineItem = std::make_shared<FixedItem>(20, 10);
    itemsControl.AddItem(inlineItem);

    itemsControl.SetItemsSource({});

    EXPECT_EQ(itemsControl.GetItemCount(), 0);
    EXPECT_TRUE(itemsControl.GetItems().empty());
    EXPECT_TRUE(itemsControl.GetVisualChildren().empty());
}

TEST(ItemsControlTests, ItemsSourceUsesTemplateToRealizeContainers)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource({std::any(1), std::any(2), std::any(3)});

    int templateCalls = 0;
    itemsControl.SetItemTemplate([&templateCalls](const std::any& data, int index) {
        ++templateCalls;
        const int value = std::any_cast<int>(data);
        return std::make_shared<FixedItem>(20 + value, 10 + index, value);
    });

    EXPECT_EQ(itemsControl.GetItemCount(), 3);
    EXPECT_EQ(templateCalls, 3);
    ASSERT_EQ(itemsControl.GetVisualChildren().size(), 3u);

    const auto first = dynamic_cast<FixedItem*>(itemsControl.GetVisualChildren()[0]);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->GetMarker(), 1);
    ASSERT_TRUE(first->GetDataContext().has_value());
    EXPECT_EQ(std::any_cast<int>(first->GetDataContext()), 1);
}

TEST(ItemsControlTests, ReplacingItemTemplateRebuildsRealizedContainers)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource({std::any(7), std::any(8)});

    itemsControl.SetItemTemplate([](const std::any&, int index) {
        return std::make_shared<FixedItem>(20, 10, index + 1);
    });
    ASSERT_EQ(itemsControl.GetVisualChildren().size(), 2u);
    itemsControl.SetItemTemplate([](const std::any&, int index) {
        return std::make_shared<FixedItem>(30, 12, 100 + index);
    });

    ASSERT_EQ(itemsControl.GetVisualChildren().size(), 2u);
    const auto newFirst = dynamic_cast<FixedItem*>(itemsControl.GetVisualChildren()[0]);
    ASSERT_NE(newFirst, nullptr);
    EXPECT_EQ(newFirst->GetMarker(), 100);
}

TEST(ItemsControlTests, ClearItemsRemovesInlineAndSourceContent)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource({std::any(1)});
    itemsControl.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<FixedItem>(20, 10);
    });
    ASSERT_EQ(itemsControl.GetItemCount(), 1);

    itemsControl.ClearItems();

    EXPECT_EQ(itemsControl.GetItemCount(), 0);
    EXPECT_TRUE(itemsControl.GetItemsSource().empty());
    EXPECT_TRUE(itemsControl.GetVisualChildren().empty());
}

TEST(ItemsControlTests, GeneratorPreparesAndCleansContainerData)
{
    NeoWPF::UI::Controls::ItemsControl::Virtualization::ItemContainerGenerator generator(nullptr);
    auto container = generator.Acquire(std::any(42), 3, [](const std::any&, int) {
        return std::make_shared<FixedItem>(20, 10);
    });

    ASSERT_NE(container, nullptr);
    ASSERT_TRUE(container->GetDataContext().has_value());
    EXPECT_EQ(std::any_cast<int>(container->GetDataContext()), 42);

    container->SetLayoutOffset(8, 12);
    generator.Recycle(3, container);

    EXPECT_FALSE(container->GetDataContext().has_value());
    EXPECT_EQ(container->GetLayoutOffsetX(), 0);
    EXPECT_EQ(container->GetLayoutOffsetY(), 0);
}

TEST(ItemsControlTests, VirtualizationRealizesTemplatedSourceItems)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetHeightMode(NeoWPF::UI::LayoutSizeMode::Auto);
    itemsControl.SetItemsSource({std::any(1), std::any(2), std::any(3), std::any(4)});
    itemsControl.SetItemTemplate([](const std::any& data, int) {
        return std::make_shared<FixedItem>(20, 10 + std::any_cast<int>(data));
    });
    itemsControl.SetVirtualization(true);

    itemsControl.Measure(100, 25);
    itemsControl.Arrange(100, 25);

    EXPECT_GT(itemsControl.GetVirtualization()->GetLastRealizedIndex(), -1);
    EXPECT_LE(itemsControl.GetVirtualization()->GetLastRealizedIndex(), 3);
    EXPECT_FALSE(itemsControl.GetVisualChildren().empty());
}

TEST(ItemsControlTests, VirtualizationMovesRealizedRangeWhenScrolling)
{
    NeoWPF::UI::ItemsControl itemsControl;
    std::vector<std::any> source;
    for (int index = 0; index < 20; ++index) {
        source.emplace_back(index);
    }

    itemsControl.SetItemsSource(source);
    itemsControl.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<FixedItem>(20, 10);
    });
    itemsControl.SetVirtualization(true);
    itemsControl.Measure(100, 20);
    itemsControl.Arrange(100, 20);

    const int initialFirst = itemsControl.GetVirtualization()->GetFirstRealizedIndex();
    itemsControl.SetVerticalOffset(120);

    EXPECT_GT(itemsControl.GetVirtualization()->GetFirstRealizedIndex(), initialFirst);
    EXPECT_GT(itemsControl.GetVirtualization()->GetVerticalOffset(), 0);
}

TEST(ItemsControlTests, ReplacingSourceClampsVirtualizationOffset)
{
    NeoWPF::UI::ItemsControl itemsControl;
    itemsControl.SetItemsSource({std::any(1), std::any(2), std::any(3), std::any(4), std::any(5)});
    itemsControl.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<FixedItem>(20, 10);
    });
    itemsControl.SetVirtualization(true);
    itemsControl.Measure(100, 20);
    itemsControl.Arrange(100, 20);
    itemsControl.SetVerticalOffset(80);
    ASSERT_GT(itemsControl.GetVirtualization()->GetVerticalOffset(), 0);

    itemsControl.SetItemsSource({std::any(9)});

    EXPECT_EQ(itemsControl.GetItemCount(), 1);
    EXPECT_EQ(itemsControl.GetVirtualization()->GetVerticalOffset(), 0);
    EXPECT_EQ(itemsControl.GetVirtualization()->GetFirstRealizedIndex(), 0);
    EXPECT_EQ(itemsControl.GetVirtualization()->GetLastRealizedIndex(), 0);
}

TEST(ItemsControlTests, DisablingVirtualizationMaterializesAllSourceItems)
{
    NeoWPF::UI::ItemsControl itemsControl;
    std::vector<std::any> source;
    for (int index = 0; index < 12; ++index) {
        source.emplace_back(index);
    }

    itemsControl.SetItemsSource(source);
    itemsControl.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<FixedItem>(20, 10);
    });
    itemsControl.SetVirtualization(true);
    itemsControl.Measure(100, 20);
    itemsControl.Arrange(100, 20);
    ASSERT_LT(itemsControl.GetVisualChildren().size(), source.size());

    itemsControl.SetVirtualization(false);

    EXPECT_EQ(itemsControl.GetVisualChildren().size(), source.size());
}

class CustomContainer final : public NeoWPF::UI::FrameworkElement {
public:
    explicit CustomContainer(int marker)
        : m_marker(marker)
    {
    }

    void Measure(double, double) override
    {
        SetDesiredSize(40, 12);
    }

    int GetMarker() const noexcept
    {
        return m_marker;
    }

private:
    int m_marker;
};

class CustomContainerItemsControl final : public NeoWPF::UI::ItemsControl {
public:
    std::shared_ptr<NeoWPF::UI::FrameworkElement> GetContainerForItem(const std::any& data, int index) override
    {
        const int value = std::any_cast<int>(data);
        return std::make_shared<CustomContainer>(value + index + 100);
    }

    void PrepareContainer(const std::shared_ptr<NeoWPF::UI::FrameworkElement>& container, const std::any& data, int index) override
    {
        container->SetDataContext(data);
        if (auto custom = dynamic_cast<CustomContainer*>(container.get())) {
            (void)custom;
            (void)index;
        }
    }
};

TEST(ItemsControlTests, OwnerProvidesContainerForSourceItems)
{
    CustomContainerItemsControl itemsControl;
    itemsControl.SetItemsSource({std::any(2), std::any(5)});
    itemsControl.SetItemTemplate([](const std::any&, int) {
        return std::make_shared<FixedItem>(15, 9, -1);
    });

    ASSERT_EQ(itemsControl.GetVisualChildren().size(), 2u);
    auto* first = dynamic_cast<CustomContainer*>(itemsControl.GetVisualChildren()[0]);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->GetMarker(), 102);

    auto* firstFramework = dynamic_cast<NeoWPF::UI::FrameworkElement*>(itemsControl.GetVisualChildren()[0]);
    ASSERT_NE(firstFramework, nullptr);
    ASSERT_TRUE(firstFramework->GetDataContext().has_value());
    EXPECT_EQ(std::any_cast<int>(firstFramework->GetDataContext()), 2);
}

} 
