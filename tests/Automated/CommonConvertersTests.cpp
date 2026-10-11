#include <gtest/gtest.h>

#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Text/FontStyle.h>

#include <string>
#include <utility>
#include <vector>

namespace {

TEST(CommonConvertersTests, EmitsCPlusPlusForSpacing)
{
    const auto converter = NeoWPF::UI::Properties::GetSpacingConverter();
    std::string expression;

    ASSERT_TRUE(converter->TryConvertToCode("5", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::Spacing(5, 5)");

    ASSERT_TRUE(converter->TryConvertToCode("5,7", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::Spacing(5, 7)");
}

using ConverterCase = std::pair<const char*, const char*>;

void ExpectCodeMappings(
    const std::shared_ptr<const NeoWPF::UI::Properties::TypeConverter>& converter,
    const std::vector<ConverterCase>& cases)
{
    ASSERT_NE(converter, nullptr);

    for (const auto& [input, expected] : cases) {
        std::string expression;
        ASSERT_TRUE(converter->TryConvertToCode(input, expression)) << input;
        EXPECT_EQ(expression, expected) << input;
    }
}

void ExpectUnknownValueRejected(
    const std::shared_ptr<const NeoWPF::UI::Properties::TypeConverter>& converter)
{
    std::string expression = "stale code";
    EXPECT_FALSE(converter->TryConvertToCode("unknown-value", expression));
    EXPECT_TRUE(expression.empty());
}

}

TEST(CommonConvertersTests, ConvertsEveryHorizontalAlignment)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetHorizontalAlignmentConverter(), {
        {"left", "NeoWPF::UI::HorizontalAlignment::Left"},
        {"center", "NeoWPF::UI::HorizontalAlignment::Center"},
        {"right", "NeoWPF::UI::HorizontalAlignment::Right"},
        {"stretch", "NeoWPF::UI::HorizontalAlignment::Stretch"},
    });
}

TEST(CommonConvertersTests, ConvertsEveryVerticalAlignment)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetVerticalAlignmentConverter(), {
        {"top", "NeoWPF::UI::VerticalAlignment::Top"},
        {"center", "NeoWPF::UI::VerticalAlignment::Center"},
        {"bottom", "NeoWPF::UI::VerticalAlignment::Bottom"},
        {"stretch", "NeoWPF::UI::VerticalAlignment::Stretch"},
    });
}

TEST(CommonConvertersTests, ConvertsEverySelectionMode)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetSelectionModeConverter(), {
        {"single", "NeoWPF::UI::SelectionMode::Single"},
        {"multiple", "NeoWPF::UI::SelectionMode::Multiple"},
        {"extended", "NeoWPF::UI::SelectionMode::Extended"},
    });
}

TEST(CommonConvertersTests, ConvertsEveryPopupPlacement)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetPopupPlacementConverter(), {
        {"bottom", "NeoWPF::UI::PopupPlacement::Bottom"},
        {"top", "NeoWPF::UI::PopupPlacement::Top"},
        {"left", "NeoWPF::UI::PopupPlacement::Left"},
        {"right", "NeoWPF::UI::PopupPlacement::Right"},
    });
}

TEST(CommonConvertersTests, ConvertsEveryFontStyle)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetFontStyleConverter(), {
        {"normal", "NeoWPF::UI::FontStyle::Normal"},
        {"italic", "NeoWPF::UI::FontStyle::Italic"},
        {"oblique", "NeoWPF::UI::FontStyle::Oblique"},
    });
}

TEST(CommonConvertersTests, ConvertsEveryFontWeight)
{
    ExpectCodeMappings(NeoWPF::UI::Properties::GetFontWeightConverter(), {
        {"thin", "100"},
        {"extralight", "200"},
        {"light", "300"},
        {"normal", "400"},
        {"medium", "500"},
        {"semibold", "600"},
        {"bold", "700"},
        {"extrabold", "800"},
        {"black", "900"},
    });
}

TEST(CommonConvertersTests, ConvertsNumericFontWeights)
{
    const auto converter = NeoWPF::UI::Properties::GetFontWeightConverter();
    std::any converted;

    ASSERT_TRUE(converter->ConvertFromString(" 700 ", converted));
    ASSERT_EQ(converted.type(), typeid(int));
    EXPECT_EQ(std::any_cast<int>(converted), 700);

    EXPECT_FALSE(converter->ConvertFromString("0", converted));
    EXPECT_FALSE(converter->ConvertFromString("1000", converted));
    EXPECT_FALSE(converter->ConvertFromString("700px", converted));
}

TEST(CommonConvertersTests, RejectsUnknownMappedValues)
{
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetHorizontalAlignmentConverter());
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetVerticalAlignmentConverter());
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetSelectionModeConverter());
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetPopupPlacementConverter());
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetFontStyleConverter());
    ExpectUnknownValueRejected(NeoWPF::UI::Properties::GetFontWeightConverter());
}

TEST(CommonConvertersTests, ConvertsBasicStringValues)
{
    using namespace NeoWPF::UI::Properties;

    std::any converted;
    ASSERT_TRUE(GetStringConverter()->ConvertFromString("hello", converted));
    ASSERT_EQ(converted.type(), typeid(std::string));
    EXPECT_EQ(std::any_cast<std::string>(converted), "hello");

    std::string expression;
    EXPECT_TRUE(GetStringConverter()->TryConvertToCode("hello", expression));
    EXPECT_EQ(expression, "\"hello\"");

    converted.reset();
    ASSERT_TRUE(GetWideStringConverter()->ConvertFromString("hello", converted));
    ASSERT_EQ(converted.type(), typeid(std::wstring));
    EXPECT_EQ(std::any_cast<std::wstring>(converted), L"hello");
    EXPECT_TRUE(GetWideStringConverter()->TryConvertToCode("hello", expression));
    EXPECT_EQ(expression, "std::wstring(L\"hello\")");
}

TEST(CommonConvertersTests, ConvertsBrushValuesToCode)
{
    std::string expression;
    const auto converter = NeoWPF::UI::Properties::GetBrushConverter();

    ASSERT_TRUE(converter->TryConvertToCode("#FF101820", expression));
    EXPECT_EQ(expression, "std::shared_ptr<NeoWPF::UI::Brush>(std::make_shared<NeoWPF::UI::SolidColorBrush>(4279244832u))");
}

TEST(CommonConvertersTests, ConvertsContentToTextBlock)
{
    std::any converted;
    const auto converter = NeoWPF::UI::Properties::GetContentConverter();

    ASSERT_TRUE(converter->ConvertFromString("hello", converted));
    ASSERT_EQ(converted.type(), typeid(std::shared_ptr<NeoWPF::UI::UIElement>));
    ASSERT_NE(std::any_cast<std::shared_ptr<NeoWPF::UI::UIElement>>(converted), nullptr);

    std::string expression;
    EXPECT_TRUE(converter->TryConvertToCode("hello", expression));
    EXPECT_EQ(expression, "std::string(\"hello\")");
}

TEST(CommonConvertersTests, ConvertsScalarValues)
{
    using namespace NeoWPF::UI::Properties;

    std::any converted;
    ASSERT_TRUE(GetIntConverter()->ConvertFromString("-42", converted));
    EXPECT_EQ(std::any_cast<int>(converted), -42);

    converted.reset();
    ASSERT_TRUE(GetFloatConverter()->ConvertFromString("1.5", converted));
    EXPECT_FLOAT_EQ(std::any_cast<float>(converted), 1.5f);

    converted.reset();
    ASSERT_TRUE(GetBoolConverter()->ConvertFromString("TRUE", converted));
    EXPECT_TRUE(std::any_cast<bool>(converted));

    std::string expression;
    EXPECT_TRUE(GetIntConverter()->TryConvertToCode("-42", expression));
    EXPECT_EQ(expression, "-42");
    EXPECT_TRUE(GetFloatConverter()->TryConvertToCode("1.5", expression));
    EXPECT_EQ(expression, "1.500000f");
    EXPECT_TRUE(GetBoolConverter()->TryConvertToCode("TRUE", expression));
    EXPECT_EQ(expression, "true");
}

TEST(CommonConvertersTests, ConvertsColorThicknessAndSpacing)
{
    using namespace NeoWPF::UI;
    using namespace NeoWPF::UI::Properties;

    std::any converted;
    ASSERT_TRUE(GetBrushConverter()->ConvertFromString("#123456", converted));
    const auto brush = std::dynamic_pointer_cast<SolidColorBrush>(
        std::any_cast<std::shared_ptr<Brush>>(converted));
    ASSERT_NE(brush, nullptr);
    EXPECT_EQ(brush->GetArgb(), 0xFF123456u);

    converted.reset();
    ASSERT_TRUE(GetThicknessConverter()->ConvertFromString("1, 2, 3, 4", converted));
    EXPECT_EQ(std::any_cast<Thickness>(converted), Thickness(1, 2, 3, 4));

    converted.reset();
    ASSERT_TRUE(GetSpacingConverter()->ConvertFromString("5, 6", converted));
    EXPECT_EQ(std::any_cast<Spacing>(converted), Spacing(5, 6));

    std::string expression;
    EXPECT_TRUE(GetThicknessConverter()->TryConvertToCode("1,2,3,4", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::Thickness(1, 2, 3, 4)");
    EXPECT_TRUE(GetBrushConverter()->TryConvertToCode("#123456", expression));
    EXPECT_EQ(expression, "std::shared_ptr<NeoWPF::UI::Brush>(std::make_shared<NeoWPF::UI::SolidColorBrush>(4279383126u))");
}
