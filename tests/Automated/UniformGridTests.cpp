#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/UniformGrid/UniformGrid.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <memory>

namespace {

class FixedElement final : public NeoWPF::UI::FrameworkElement {
public:
    FixedElement(double width, double height)
        : m_width(width), m_height(height)
    {
        SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Left);
        SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Top);
    }

    void Measure(double, double) override
    {
        SetDesiredSize(m_width, m_height);
    }

private:
    double m_width;
    double m_height;
};

TEST(UniformGridTests, DefaultsAndDependencyProperties)
{
    NeoWPF::UI::UniformGrid grid;

    EXPECT_EQ(grid.GetRows(), 0);
    EXPECT_EQ(grid.GetColumns(), 0);
    EXPECT_EQ(grid.GetFirstColumn(), 0);

    grid.SetRows(2);
    grid.SetColumns(3);
    grid.SetFirstColumn(1);

    EXPECT_EQ(grid.GetRows(), 2);
    EXPECT_EQ(grid.GetColumns(), 3);
    EXPECT_EQ(grid.GetFirstColumn(), 1);
}

TEST(UniformGridTests, AutomaticDimensionsUseNearSquareRowMajorCells)
{
    NeoWPF::UI::UniformGrid grid;
    auto first = std::make_shared<FixedElement>(10.0, 8.0);
    auto second = std::make_shared<FixedElement>(12.0, 9.0);
    auto third = std::make_shared<FixedElement>(14.0, 10.0);
    auto fourth = std::make_shared<FixedElement>(16.0, 11.0);
    grid.AddChild(first);
    grid.AddChild(second);
    grid.AddChild(third);
    grid.AddChild(fourth);

    grid.Measure(120.0, 80.0);
    grid.Arrange(120.0, 80.0);

    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 60.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetY(), 40.0);
    EXPECT_DOUBLE_EQ(fourth->GetLayoutOffsetX(), 60.0);
    EXPECT_DOUBLE_EQ(fourth->GetLayoutOffsetY(), 40.0);
}

TEST(UniformGridTests, InfersColumnsFromConfiguredRows)
{
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(3);
    for (int index = 0; index < 4; ++index) {
        grid.AddChild(std::make_shared<FixedElement>(10.0, 8.0));
    }

    grid.Measure(90.0, 60.0);
    grid.Arrange(90.0, 60.0);

    const auto children = grid.GetVisualChildren();
    ASSERT_EQ(children.size(), 4u);
    auto* third = dynamic_cast<NeoWPF::UI::FrameworkElement*>(children[2]);
    auto* fourth = dynamic_cast<NeoWPF::UI::FrameworkElement*>(children[3]);
    ASSERT_NE(third, nullptr);
    ASSERT_NE(fourth, nullptr);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetY(), 20.0);
    EXPECT_DOUBLE_EQ(fourth->GetLayoutOffsetX(), 45.0);
    EXPECT_DOUBLE_EQ(fourth->GetLayoutOffsetY(), 20.0);
}

TEST(UniformGridTests, FirstColumnAndOverflowPreserveAllChildren)
{
    NeoWPF::Diagnostics::ClearDeveloperWarnings();
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(1);
    grid.SetColumns(2);
    grid.SetFirstColumn(1);
    auto first = std::make_shared<FixedElement>(10.0, 8.0);
    auto second = std::make_shared<FixedElement>(12.0, 9.0);
    auto third = std::make_shared<FixedElement>(14.0, 10.0);
    grid.AddChild(first);
    grid.AddChild(second);
    grid.AddChild(third);

    grid.Measure(100.0, 80.0);
    grid.Arrange(100.0, 80.0);

    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 50.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 40.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetX(), 50.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetY(), 40.0);
    ASSERT_EQ(grid.GetVisualChildren().size(), 3u);
    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_NE(warning->title.find(L"NEO-UI209"), std::wstring::npos);
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
}

TEST(UniformGridTests, InvisibleChildrenDoNotConsumeCells)
{
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(1);
    grid.SetColumns(2);
    auto hidden = std::make_shared<FixedElement>(90.0, 90.0);
    auto visible = std::make_shared<FixedElement>(10.0, 8.0);
    hidden->SetVisibility(false);
    grid.AddChild(hidden);
    grid.AddChild(visible);

    grid.Measure(100.0, 40.0);
    grid.Arrange(100.0, 40.0);

    EXPECT_DOUBLE_EQ(visible->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(visible->GetLayoutOffsetY(), 0.0);
}

TEST(UniformGridTests, AppliesMarginsAndAlignmentWithinCells)
{
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(1);
    grid.SetColumns(2);
    auto child = std::make_shared<FixedElement>(10.0, 10.0);
    child->SetMargin(2.0, 3.0, 4.0, 5.0);
    child->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    child->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    grid.AddChild(child);

    grid.Measure(100.0, 40.0);
    grid.Arrange(100.0, 40.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 19.0);
    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetY(), 14.0);
    EXPECT_DOUBLE_EQ(child->GetFinalWidth(), 10.0);
    EXPECT_DOUBLE_EQ(child->GetFinalHeight(), 10.0);
}

TEST(UniformGridTests, NegativeDimensionsUseAutomaticSizingAndReportError)
{
    NeoWPF::Diagnostics::ClearDeveloperWarnings();
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(-1);
    grid.SetColumns(2);
    grid.AddChild(std::make_shared<FixedElement>(10.0, 8.0));
    grid.AddChild(std::make_shared<FixedElement>(12.0, 9.0));

    grid.Measure(100.0, 40.0);
    grid.Arrange(100.0, 40.0);

    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_NE(warning->title.find(L"NEO-UI210"), std::wstring::npos);
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
}

TEST(UniformGridTests, ClampsInvalidFirstColumnAndReportsError)
{
    NeoWPF::Diagnostics::ClearDeveloperWarnings();
    NeoWPF::UI::UniformGrid grid;
    grid.SetRows(1);
    grid.SetColumns(2);
    grid.SetFirstColumn(8);
    auto child = std::make_shared<FixedElement>(10.0, 8.0);
    grid.AddChild(child);

    grid.Measure(100.0, 40.0);
    grid.Arrange(100.0, 40.0);

    EXPECT_DOUBLE_EQ(child->GetLayoutOffsetX(), 50.0);
    const auto warning = NeoWPF::Diagnostics::GetLastWarning();
    ASSERT_TRUE(warning.has_value());
    EXPECT_NE(warning->title.find(L"NEO-UI211"), std::wstring::npos);
    EXPECT_EQ(warning->severity, NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
}

TEST(UniformGridTests, RegistersAndParsesXamlPropertiesAndChildren)
{
    NeoWPF::Core::Application::Initialize();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <UniformGrid Rows="2" Columns="3" FirstColumn="1">
            <TextBlock Text="First" />
            <TextBlock Text="Second" />
        </UniformGrid>
    )");

    ASSERT_EQ(roots.size(), 1u);
    const auto tree = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    auto grid = std::dynamic_pointer_cast<NeoWPF::UI::UniformGrid>(tree);
    ASSERT_NE(grid, nullptr);
    EXPECT_EQ(grid->GetRows(), 2);
    EXPECT_EQ(grid->GetColumns(), 3);
    EXPECT_EQ(grid->GetFirstColumn(), 1);
    EXPECT_EQ(grid->GetVisualChildren().size(), 2u);

    NeoWPF::Core::Application::Shutdown();
}

}