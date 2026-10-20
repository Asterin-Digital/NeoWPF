#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Layout/Canvas/Canvas.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/UniformGrid/UniformGrid.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Text/FontStyle.h>

#include <any>
#include <memory>
#include <string>
#include <typeinfo>

namespace {

class ApplicationScope {
public:
    ApplicationScope()
    {
        NeoWPF::Core::Application::Initialize();
    }

    ~ApplicationScope()
    {
        NeoWPF::Core::Application::Shutdown();
    }
};

template<typename TElement, typename TValue>
void ExpectProperty(const std::string& xaml,
                    const std::string& propertyName,
                    const TValue& expectedValue)
{
    ApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(xaml);
    ASSERT_EQ(roots.size(), 1u);

    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    ASSERT_NE(object, nullptr);

    const auto element = std::dynamic_pointer_cast<TElement>(object);
    ASSERT_NE(element, nullptr);

    const auto* property = element->FindDependencyProperty(propertyName);
    ASSERT_NE(property, nullptr);

    const auto value = element->GetRawValue(*property);
    ASSERT_EQ(value.type(), typeid(TValue));
    EXPECT_EQ(std::any_cast<TValue>(value), expectedValue);
}

template<typename TElement>
void ExpectBrush(const std::string& xaml,
                 const std::string& propertyName,
                 std::uint32_t expectedArgb)
{
    ApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(xaml);
    ASSERT_EQ(roots.size(), 1u);

    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    ASSERT_NE(object, nullptr);

    const auto element = std::dynamic_pointer_cast<TElement>(object);
    ASSERT_NE(element, nullptr);

    const auto* property = element->FindDependencyProperty(propertyName);
    ASSERT_NE(property, nullptr);

    const auto value = element->GetRawValue(*property);
    ASSERT_EQ(value.type(), typeid(std::shared_ptr<NeoWPF::UI::Brush>));
    const auto brush = std::any_cast<std::shared_ptr<NeoWPF::UI::Brush>>(value);
    ASSERT_NE(brush, nullptr);

    const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush);
    ASSERT_NE(solidBrush, nullptr);
    EXPECT_EQ(solidBrush->GetArgb(), expectedArgb);
}

template<typename TElement, typename TGetter>
void ExpectInvalidValueKeepsDefault(const std::string& xaml,
                                    const std::string& propertyName,
                                    TGetter getter)
{
    ApplicationScope application;

    const auto defaultElement = std::make_shared<TElement>();
    const auto* defaultProperty = defaultElement->FindDependencyProperty(propertyName);
    ASSERT_NE(defaultProperty, nullptr);
    const auto defaultValue = getter(*defaultElement);

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(xaml);
    ASSERT_EQ(roots.size(), 1u);

    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    ASSERT_NE(object, nullptr);

    const auto element = std::dynamic_pointer_cast<TElement>(object);
    ASSERT_NE(element, nullptr);

    const auto* property = element->FindDependencyProperty(propertyName);
    ASSERT_NE(property, nullptr);

    EXPECT_EQ(getter(*element), defaultValue);
}

template<typename TParent, typename TChild, typename TGetter, typename TValue>
void ExpectChildValue(const std::string& xaml, TGetter getter, const TValue& expectedValue)
{
    ApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(xaml);
    ASSERT_EQ(roots.size(), 1u);

    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    ASSERT_NE(object, nullptr);

    const auto parent = std::dynamic_pointer_cast<TParent>(object);
    ASSERT_NE(parent, nullptr);

    const auto children = parent->GetVisualChildren();
    ASSERT_EQ(children.size(), 1u);
    auto* child = dynamic_cast<TChild*>(children[0]);
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(getter(*parent, child), expectedValue);
}

}

TEST(XamlElementPropertyTests, ParsesWidth)
{
    ExpectProperty<NeoWPF::UI::StackPanel>(R"(<StackPanel Width="120" />)", "Width", 120.0);
}

TEST(XamlElementPropertyTests, ParsesHeight)
{
    ExpectProperty<NeoWPF::UI::StackPanel>(R"(<StackPanel Height="45" />)", "Height", 45.0);
}

TEST(XamlElementPropertyTests, ParsesWidthMode)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border WidthMode="Stretch" />)",
                                       "WidthMode",
                                       NeoWPF::UI::LayoutSizeMode::Stretch);
}

TEST(XamlElementPropertyTests, ParsesHeightMode)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border HeightMode="Auto" />)",
                                       "HeightMode",
                                       NeoWPF::UI::LayoutSizeMode::Auto);
}

TEST(XamlElementPropertyTests, ParsesHorizontalAlignment)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border HorizontalAlignment="Right" />)",
                                       "HorizontalAlignment",
                                       NeoWPF::UI::HorizontalAlignment::Right);
}

TEST(XamlElementPropertyTests, ParsesVerticalAlignment)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border VerticalAlignment="Bottom" />)",
                                       "VerticalAlignment",
                                       NeoWPF::UI::VerticalAlignment::Bottom);
}

TEST(XamlElementPropertyTests, ParsesMargin)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border Margin="1,2,3,4" />)",
                                       "Margin",
                                       NeoWPF::UI::Thickness(1, 2, 3, 4));
}

TEST(XamlElementPropertyTests, ParsesSpacing)
{
    ExpectProperty<NeoWPF::UI::StackPanel>(R"(<StackPanel Spacing="3,5" />)",
                                           "Spacing",
                                           NeoWPF::UI::Spacing(3, 5));
}

TEST(XamlElementPropertyTests, ParsesCornerRadius)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border CornerRadius="1,2,3,4" />)",
                                       "CornerRadius",
                                       NeoWPF::UI::CornerRadius(1, 2, 3, 4));
}

TEST(XamlElementPropertyTests, ParsesClipToBounds)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border ClipToBounds="True" />)",
                                       "ClipToBounds",
                                       true);
}

TEST(XamlElementPropertyTests, ParsesOpacity)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border Opacity="0.5" />)", "Opacity", 0.5f);
}

TEST(XamlElementPropertyTests, ParsesBorderThickness)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border BorderThickness="1,2,3,4" />)",
                                       "BorderThickness",
                                       NeoWPF::UI::Thickness(1, 2, 3, 4));
}

TEST(XamlElementPropertyTests, ParsesPadding)
{
    ExpectProperty<NeoWPF::UI::Border>(R"(<Border Padding="4,3,2,1" />)",
                                       "Padding",
                                       NeoWPF::UI::Thickness(4, 3, 2, 1));
}

TEST(XamlElementPropertyTests, ParsesBackgroundColor)
{
    ExpectBrush<NeoWPF::UI::Button>(R"(<Button Background="#FF112233" />)",
                                    "Background",
                                    0xFF112233u);
}

TEST(XamlElementPropertyTests, ParsesBorderBrushColor)
{
    ExpectBrush<NeoWPF::UI::Button>(R"(<Button BorderBrush="#FF445566" />)",
                                    "BorderBrush",
                                    0xFF445566u);
}

TEST(XamlElementPropertyTests, ParsesTextForegroundColor)
{
    ExpectBrush<NeoWPF::UI::TextBlock>(R"(<TextBlock Foreground="#FF778899" />)",
                                       "Foreground",
                                       0xFF778899u);
}

TEST(XamlElementPropertyTests, ParsesFontSize)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock FontSize="18" />)", "FontSize", 18);
}

TEST(XamlElementPropertyTests, ParsesFontFamily)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock FontFamily="Fira Code" />)",
                                          "FontFamily",
                                          std::wstring(L"Fira Code"));
}

TEST(XamlElementPropertyTests, ParsesFontWeight)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock FontWeight="Bold" />)", "FontWeight", 700);
}

TEST(XamlElementPropertyTests, ParsesFontStyle)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock FontStyle="Italic" />)",
                                          "FontStyle",
                                          NeoWPF::UI::FontStyle::Italic);
}

TEST(XamlElementPropertyTests, ParsesTextHorizontalAlignment)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock TextHorizontalAlignment="Center" />)",
                                          "TextHorizontalAlignment",
                                          NeoWPF::UI::TextHorizontalAlignment::Center);
}

TEST(XamlElementPropertyTests, ParsesTextVerticalAlignment)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock TextVerticalAlignment="Center" />)",
                                          "TextVerticalAlignment",
                                          NeoWPF::UI::TextVerticalAlignment::Center);
}

TEST(XamlElementPropertyTests, ParsesTextWrapping)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock TextWrapping="Wrap" />)",
                                          "TextWrapping",
                                          NeoWPF::UI::TextWrapping::Wrap);
}

TEST(XamlElementPropertyTests, ParsesTextPadding)
{
    ExpectProperty<NeoWPF::UI::TextBlock>(R"(<TextBlock Padding="2" />)",
                                          "Padding",
                                          NeoWPF::UI::Thickness(2));
}

TEST(XamlElementPropertyTests, ParsesStackPanelOrientation)
{
    ExpectProperty<NeoWPF::UI::StackPanel>(R"(<StackPanel Orientation="Horizontal" />)",
                                           "Orientation",
                                           static_cast<int>(NeoWPF::UI::Orientation::Horizontal));
}

TEST(XamlElementPropertyTests, ParsesGridRow)
{
    ExpectChildValue<NeoWPF::UI::Grid, NeoWPF::UI::Border>(
        R"(<Grid><Border Grid.Row="2" /></Grid>)",
        [](const auto& grid, auto* child) { return grid.GetRow(child); },
        2);
}

TEST(XamlElementPropertyTests, ParsesGridColumn)
{
    ExpectChildValue<NeoWPF::UI::Grid, NeoWPF::UI::Border>(
        R"(<Grid><Border Grid.Column="3" /></Grid>)",
        [](const auto& grid, auto* child) { return grid.GetColumn(child); },
        3);
}

TEST(XamlElementPropertyTests, ParsesGridRowSpan)
{
    ExpectChildValue<NeoWPF::UI::Grid, NeoWPF::UI::Border>(
        R"(<Grid><Border Grid.RowSpan="2" /></Grid>)",
        [](const auto&, auto* child) { return NeoWPF::UI::Grid::GetRowSpan(child); },
        2);
}

TEST(XamlElementPropertyTests, ParsesGridColumnSpan)
{
    ExpectChildValue<NeoWPF::UI::Grid, NeoWPF::UI::Border>(
        R"(<Grid><Border Grid.ColumnSpan="4" /></Grid>)",
        [](const auto&, auto* child) { return NeoWPF::UI::Grid::GetColumnSpan(child); },
        4);
}

TEST(XamlElementPropertyTests, ParsesCanvasLeft)
{
    ExpectChildValue<NeoWPF::UI::Canvas, NeoWPF::UI::Border>(
        R"(<Canvas><Border Canvas.Left="10" /></Canvas>)",
        [](const auto&, const auto* child) { return NeoWPF::UI::Canvas::GetLeft(child); },
        10.0);
}

TEST(XamlElementPropertyTests, ParsesCanvasTop)
{
    ExpectChildValue<NeoWPF::UI::Canvas, NeoWPF::UI::Border>(
        R"(<Canvas><Border Canvas.Top="20" /></Canvas>)",
        [](const auto&, const auto* child) { return NeoWPF::UI::Canvas::GetTop(child); },
        20.0);
}

TEST(XamlElementPropertyTests, ParsesCanvasRight)
{
    ExpectChildValue<NeoWPF::UI::Canvas, NeoWPF::UI::Border>(
        R"(<Canvas><Border Canvas.Right="30" /></Canvas>)",
        [](const auto&, const auto* child) { return NeoWPF::UI::Canvas::GetRight(child); },
        30.0);
}

TEST(XamlElementPropertyTests, ParsesCanvasBottom)
{
    ExpectChildValue<NeoWPF::UI::Canvas, NeoWPF::UI::Border>(
        R"(<Canvas><Border Canvas.Bottom="40" /></Canvas>)",
        [](const auto&, const auto* child) { return NeoWPF::UI::Canvas::GetBottom(child); },
        40.0);
}

TEST(XamlElementPropertyTests, ParsesGridRowHeight)
{
    ApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<RowDefinition Height="24" />)");
    ASSERT_EQ(roots.size(), 1u);
    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto row = std::dynamic_pointer_cast<NeoWPF::UI::RowDefinition>(object);
    ASSERT_NE(row, nullptr);
    EXPECT_EQ(row->GetHeight().Mode, NeoWPF::UI::SizeMode::Fixed);
    EXPECT_EQ(row->GetHeight().Value, 24);
}

TEST(XamlElementPropertyTests, ParsesGridColumnWidth)
{
    ApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(
        R"(<ColumnDefinition Width="2*" />)");
    ASSERT_EQ(roots.size(), 1u);
    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto column = std::dynamic_pointer_cast<NeoWPF::UI::ColumnDefinition>(object);
    ASSERT_NE(column, nullptr);
    EXPECT_EQ(column->GetWidth().Mode, NeoWPF::UI::SizeMode::Star);
    EXPECT_EQ(column->GetWidth().Value, 2);
}

TEST(XamlElementPropertyTests, ParsesUniformGridRows)
{
    ExpectProperty<NeoWPF::UI::UniformGrid>(R"(<UniformGrid Rows="2" />)", "Rows", 2);
}

TEST(XamlElementPropertyTests, ParsesUniformGridColumns)
{
    ExpectProperty<NeoWPF::UI::UniformGrid>(R"(<UniformGrid Columns="3" />)", "Columns", 3);
}

TEST(XamlElementPropertyTests, ParsesUniformGridFirstColumn)
{
    ExpectProperty<NeoWPF::UI::UniformGrid>(R"(<UniformGrid FirstColumn="1" />)", "FirstColumn", 1);
}

TEST(XamlElementPropertyTests, InvalidCornerRadiusKeepsDefault)
{
    ExpectInvalidValueKeepsDefault<NeoWPF::UI::Border>(
        R"(<Border CornerRadius="1,2,3" />)",
        "CornerRadius",
        [](const auto& border) { return border.GetCornerRadius(); });
}

TEST(XamlElementPropertyTests, InvalidOpacityKeepsDefault)
{
    ExpectInvalidValueKeepsDefault<NeoWPF::UI::Border>(
        R"(<Border Opacity="not-a-number" />)",
        "Opacity",
        [](const auto& border) { return border.GetOpacity(); });
}

TEST(XamlElementPropertyTests, InvalidTextWrappingKeepsDefault)
{
    ExpectInvalidValueKeepsDefault<NeoWPF::UI::TextBlock>(
        R"(<TextBlock TextWrapping="Diagonal" />)",
        "TextWrapping",
        [](const auto& textBlock) { return textBlock.GetTextWrapping(); });
}