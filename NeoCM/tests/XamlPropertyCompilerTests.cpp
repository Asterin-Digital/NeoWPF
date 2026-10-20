#include <gtest/gtest.h>

#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Core/Compiler/ResolvedXaml.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <string>
#include <ostream>

namespace NeoCM::Core::Compiler {
namespace {

struct PropertyCase {
    const char* Name;
    const char* Xaml;
    const char* PropertyName;
    const char* PropertyExpression;
};

void PrintTo(const PropertyCase& propertyCase, std::ostream* output)
{
    *output << propertyCase.Name;
}

class XamlPropertyCompilerTests : public testing::TestWithParam<PropertyCase> {};

TEST_P(XamlPropertyCompilerTests, ResolvesAndConvertsProperty)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    const auto& propertyCase = GetParam();
    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(propertyCase.Xaml);
    const auto resolved = ResolveXaml(roots, "Property.xaml");

    ASSERT_TRUE(resolved.success) << resolved.error;
    const auto& properties = resolved.document.root.properties;
    ASSERT_EQ(properties.size(), 1u);
    EXPECT_EQ(properties[0].name, propertyCase.PropertyName);
    EXPECT_EQ(properties[0].propertyExpression, propertyCase.PropertyExpression);
    EXPECT_FALSE(properties[0].valueExpression.empty());
}

INSTANTIATE_TEST_SUITE_P(
    StylingAndLayout,
    XamlPropertyCompilerTests,
    testing::Values(
        PropertyCase{"Width", R"(<Border Width="120" />)", "Width", "NeoWPF::UI::FrameworkElement::WidthProperty()"},
        PropertyCase{"Height", R"(<Border Height="45" />)", "Height", "NeoWPF::UI::FrameworkElement::HeightProperty()"},
        PropertyCase{"WidthMode", R"(<Border WidthMode="Stretch" />)", "WidthMode", "NeoWPF::UI::FrameworkElement::WidthModeProperty()"},
        PropertyCase{"HeightMode", R"(<Border HeightMode="Auto" />)", "HeightMode", "NeoWPF::UI::FrameworkElement::HeightModeProperty()"},
        PropertyCase{"HorizontalAlignment", R"(<Border HorizontalAlignment="Right" />)", "HorizontalAlignment", "NeoWPF::UI::FrameworkElement::HorizontalAlignmentProperty()"},
        PropertyCase{"VerticalAlignment", R"(<Border VerticalAlignment="Bottom" />)", "VerticalAlignment", "NeoWPF::UI::FrameworkElement::VerticalAlignmentProperty()"},
        PropertyCase{"Margin", R"(<Border Margin="1,2,3,4" />)", "Margin", "NeoWPF::UI::FrameworkElement::MarginProperty()"},
        PropertyCase{"Spacing", R"(<StackPanel Spacing="3,5" />)", "Spacing", "NeoWPF::UI::FrameworkElement::SpacingProperty()"},
        PropertyCase{"CornerRadius", R"(<Border CornerRadius="1,2,3,4" />)", "CornerRadius", "NeoWPF::UI::FrameworkElement::CornerRadiusProperty()"},
        PropertyCase{"ClipToBounds", R"(<Border ClipToBounds="True" />)", "ClipToBounds", "NeoWPF::UI::FrameworkElement::ClipToBoundsProperty()"},
        PropertyCase{"Opacity", R"(<Border Opacity="0.5" />)", "Opacity", "NeoWPF::UI::UIElement::OpacityProperty()"},
        PropertyCase{"BorderThickness", R"(<Border BorderThickness="1,2,3,4" />)", "BorderThickness", "NeoWPF::UI::Border::BorderThicknessProperty()"},
        PropertyCase{"BorderPadding", R"(<Border Padding="4,3,2,1" />)", "Padding", "NeoWPF::UI::Border::PaddingProperty()"},
        PropertyCase{"Background", R"(<Button Background="#FF112233" />)", "Background", "NeoWPF::UI::Control::BackgroundProperty()"},
        PropertyCase{"BorderBrush", R"(<Button BorderBrush="#FF445566" />)", "BorderBrush", "NeoWPF::UI::Control::BorderBrushProperty()"},
        PropertyCase{"Foreground", R"(<TextBlock Foreground="#FF778899" />)", "Foreground", "NeoWPF::UI::TextBlock::ForegroundProperty()"},
        PropertyCase{"FontSize", R"(<TextBlock FontSize="18" />)", "FontSize", "NeoWPF::UI::TextBlock::FontSizeProperty()"},
        PropertyCase{"FontFamily", R"(<TextBlock FontFamily="Fira Code" />)", "FontFamily", "NeoWPF::UI::TextBlock::FontFamilyProperty()"},
        PropertyCase{"FontWeight", R"(<TextBlock FontWeight="Bold" />)", "FontWeight", "NeoWPF::UI::TextBlock::FontWeightProperty()"},
        PropertyCase{"FontStyle", R"(<TextBlock FontStyle="Italic" />)", "FontStyle", "NeoWPF::UI::TextBlock::FontStyleProperty()"},
        PropertyCase{"TextHorizontalAlignment", R"(<TextBlock TextHorizontalAlignment="Center" />)", "TextHorizontalAlignment", "NeoWPF::UI::TextBlock::TextHorizontalAlignmentProperty()"},
        PropertyCase{"TextVerticalAlignment", R"(<TextBlock TextVerticalAlignment="Center" />)", "TextVerticalAlignment", "NeoWPF::UI::TextBlock::TextVerticalAlignmentProperty()"},
        PropertyCase{"TextWrapping", R"(<TextBlock TextWrapping="Wrap" />)", "TextWrapping", "NeoWPF::UI::TextBlock::TextWrappingProperty()"},
        PropertyCase{"TextPadding", R"(<TextBlock Padding="2" />)", "Padding", "NeoWPF::UI::TextBlock::PaddingProperty()"},
        PropertyCase{"Orientation", R"(<StackPanel Orientation="Horizontal" />)", "Orientation", "NeoWPF::UI::StackPanel::OrientationProperty()"},
        PropertyCase{"GridRow", R"(<Border Grid.Row="2" />)", "Grid.Row", "NeoWPF::UI::Grid::RowProperty()"},
        PropertyCase{"GridColumn", R"(<Border Grid.Column="3" />)", "Grid.Column", "NeoWPF::UI::Grid::ColumnProperty()"},
        PropertyCase{"GridRowSpan", R"(<Border Grid.RowSpan="2" />)", "Grid.RowSpan", "NeoWPF::UI::Grid::RowSpanProperty()"},
        PropertyCase{"GridColumnSpan", R"(<Border Grid.ColumnSpan="4" />)", "Grid.ColumnSpan", "NeoWPF::UI::Grid::ColumnSpanProperty()"},
        PropertyCase{"CanvasLeft", R"(<Border Canvas.Left="10" />)", "Canvas.Left", "NeoWPF::UI::Canvas::LeftProperty()"},
        PropertyCase{"CanvasTop", R"(<Border Canvas.Top="20" />)", "Canvas.Top", "NeoWPF::UI::Canvas::TopProperty()"},
        PropertyCase{"CanvasRight", R"(<Border Canvas.Right="30" />)", "Canvas.Right", "NeoWPF::UI::Canvas::RightProperty()"},
        PropertyCase{"CanvasBottom", R"(<Border Canvas.Bottom="40" />)", "Canvas.Bottom", "NeoWPF::UI::Canvas::BottomProperty()"},
        PropertyCase{"RowHeight", R"(<RowDefinition Height="24" />)", "Height", "NeoWPF::UI::RowDefinition::HeightProperty()"},
        PropertyCase{"ColumnWidth", R"(<ColumnDefinition Width="2*" />)", "Width", "NeoWPF::UI::ColumnDefinition::WidthProperty()"},
        PropertyCase{"UniformGridRows", R"(<UniformGrid Rows="2" />)", "Rows", "NeoWPF::UI::UniformGrid::RowsProperty()"},
        PropertyCase{"UniformGridColumns", R"(<UniformGrid Columns="3" />)", "Columns", "NeoWPF::UI::UniformGrid::ColumnsProperty()"},
        PropertyCase{"UniformGridFirstColumn", R"(<UniformGrid FirstColumn="1" />)", "FirstColumn", "NeoWPF::UI::UniformGrid::FirstColumnProperty()"}),
    [](const testing::TestParamInfo<PropertyCase>& info) {
        return std::string(info.param.Name);
    });

struct InvalidPropertyCase {
    const char* Name;
    const char* Xaml;
    const char* PropertyName;
};

void PrintTo(const InvalidPropertyCase& propertyCase, std::ostream* output)
{
    *output << propertyCase.Name;
}

class InvalidXamlPropertyCompilerTests : public testing::TestWithParam<InvalidPropertyCase> {};

TEST_P(InvalidXamlPropertyCompilerTests, RejectsInvalidValue)
{
    const auto& propertyCase = GetParam();
    const auto result = CompileXaml(propertyCase.Xaml, "InvalidProperty.xaml");

    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find(std::string("invalid value for property '") +
                                propertyCase.PropertyName + "'"),
              std::string::npos);
}

INSTANTIATE_TEST_SUITE_P(
    StylingAndLayout,
    InvalidXamlPropertyCompilerTests,
    testing::Values(
        InvalidPropertyCase{"CornerRadius", R"(<Border CornerRadius="1,2,3" />)", "CornerRadius"},
        InvalidPropertyCase{"Opacity", R"(<Border Opacity="not-a-number" />)", "Opacity"},
        InvalidPropertyCase{"TextWrapping", R"(<TextBlock TextWrapping="Diagonal" />)", "TextWrapping"},
        InvalidPropertyCase{"WidthMode", R"(<Border WidthMode="Diagonal" />)", "WidthMode"},
        InvalidPropertyCase{"Orientation", R"(<StackPanel Orientation="Diagonal" />)", "Orientation"}),
    [](const testing::TestParamInfo<InvalidPropertyCase>& info) {
        return std::string(info.param.Name);
    });

}
}