#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Rendering/Text/FontDatabase.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
#include <NeoWPF/UI/Text/TextFlow.h>

#include <algorithm>
#include <string>

namespace {

constexpr int kFontSize = 16;
constexpr int kFontWeight = 400;
constexpr auto kFontFamily = L"NeoWPF TextFlow Tests";
constexpr auto kFontStyle = NeoWPF::UI::FontStyle::Normal;

bool RegisterTextTestFont()
{
    static const bool registered = [] {
        for (std::size_t index = 0;
             index < NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount();
             ++index) {
            NeoWPF::Core::ApplicationResources::Embedded::FontRecord record;
            if (NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont(index, record) &&
                record.data && record.size > 0 &&
                NeoWPF::Rendering::Text::RegisterFontMemory(record.data, record.size, kFontFamily)) {
                return true;
            }
        }
        return false;
    }();
    return registered;
}

NeoWPF::UI::Text::FlowResult BreakText(
    const std::wstring& text,
    double width,
    NeoWPF::UI::TextWrapping wrapping)
{
    return NeoWPF::UI::Text::BreakText(
        text, kFontSize, kFontFamily, width, wrapping, kFontWeight, kFontStyle);
}

TEST(TextFlowTests, NoWrapPreservesTheLogicalLine)
{
    ASSERT_TRUE(RegisterTextTestFont());
    const auto flow = BreakText(L"alpha beta", 1.0, NeoWPF::UI::TextWrapping::NoWrap);

    ASSERT_EQ(flow.lines.size(), 1u);
    EXPECT_EQ(flow.lines.front().text, L"alpha beta");
}

TEST(TextFlowTests, ExplicitNewLinesPreserveSeparateAndTrailingEmptyLines)
{
    ASSERT_TRUE(RegisterTextTestFont());
    const auto flow = BreakText(L"first\nsecond\n", 0.0, NeoWPF::UI::TextWrapping::NoWrap);

    ASSERT_EQ(flow.lines.size(), 3u);
    EXPECT_EQ(flow.lines[0].text, L"first");
    EXPECT_EQ(flow.lines[1].text, L"second");
    EXPECT_TRUE(flow.lines[2].text.empty());
}

TEST(TextFlowTests, WrapSplitsAnOversizedWordWithoutLosingCharacters)
{
    ASSERT_TRUE(RegisterTextTestFont());
    const std::wstring word = L"WWWWWWWWWW";
    const auto fullWidth = NeoWPF::Rendering::Text::MeasureText(
        word, kFontSize, kFontFamily, 0, 0, kFontWeight, kFontStyle).width;
    ASSERT_GT(fullWidth, 0);

    const auto flow = BreakText(word, fullWidth / 2.0, NeoWPF::UI::TextWrapping::Wrap);

    ASSERT_GT(flow.lines.size(), 1u);
    std::wstring reconstructed;
    for (const auto& line : flow.lines) {
        reconstructed += line.text;
    }
    EXPECT_EQ(reconstructed, word);
}

TEST(TextFlowTests, WrapWithOverflowKeepsAnOversizedWordOnOneLine)
{
    ASSERT_TRUE(RegisterTextTestFont());
    const std::wstring word = L"WWWWWWWWWW";
    const auto fullWidth = NeoWPF::Rendering::Text::MeasureText(
        word, kFontSize, kFontFamily, 0, 0, kFontWeight, kFontStyle).width;
    ASSERT_GT(fullWidth, 0);

    const auto flow = BreakText(
        word, fullWidth / 2.0, NeoWPF::UI::TextWrapping::WrapWithOverflow);

    ASSERT_EQ(flow.lines.size(), 1u);
    EXPECT_EQ(flow.lines.front().text, word);
}

TEST(TextFlowTests, WrapBreaksAtWhitespaceWithoutReorderingWords)
{
    ASSERT_TRUE(RegisterTextTestFont());
    const auto firstWidth = NeoWPF::Rendering::Text::MeasureText(
        L"alpha", kFontSize, kFontFamily, 0, 0, kFontWeight, kFontStyle).width;
    const auto secondWidth = NeoWPF::Rendering::Text::MeasureText(
        L"beta", kFontSize, kFontFamily, 0, 0, kFontWeight, kFontStyle).width;
    ASSERT_GT(firstWidth, 0);
    ASSERT_GT(secondWidth, 0);
    const double width = (std::max)(firstWidth, secondWidth);

    const auto flow = BreakText(L"alpha beta", width, NeoWPF::UI::TextWrapping::Wrap);

    ASSERT_EQ(flow.lines.size(), 2u);
    EXPECT_EQ(flow.lines[0].text, L"alpha ");
    EXPECT_EQ(flow.lines[1].text, L"beta");
}

TEST(TextFlowTests, ArrangeAtMeasuredWidthPreservesMeasuredFlowDimensions)
{
    ASSERT_TRUE(RegisterTextTestFont());
    NeoWPF::UI::TextBlock textBlock(L"WWWWWWWWWW WWWWWWWWWW");
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(90.0, 240.0);
    const double measuredHeight = textBlock.GetDesiredHeight();

    textBlock.Arrange(90.0, measuredHeight);

    EXPECT_DOUBLE_EQ(textBlock.GetContentHeight(), measuredHeight);
}

TEST(TextFlowTests, ArrangeAtDifferentWidthReflowsMeasuredText)
{
    ASSERT_TRUE(RegisterTextTestFont());
    NeoWPF::UI::TextBlock textBlock(L"WWWWWWWWWW WWWWWWWWWW");
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(240.0, 240.0);
    const double measuredHeight = textBlock.GetDesiredHeight();

    textBlock.Arrange(45.0, measuredHeight);

    EXPECT_GT(textBlock.GetContentHeight(), measuredHeight);
}

TEST(TextFlowTests, TextChangeInvalidatesMeasuredFlow)
{
    ASSERT_TRUE(RegisterTextTestFont());
    NeoWPF::UI::TextBlock textBlock(L"short");
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(120.0, 240.0);
    const double shortHeight = textBlock.GetDesiredHeight();

    textBlock.SetText(L"WWWWWWWWWW\nWWWWWWWWWW\nWWWWWWWWWW");
    textBlock.Measure(120.0, 240.0);

    EXPECT_GT(textBlock.GetDesiredHeight(), shortHeight);
}

TEST(TextFlowTests, FontSizeChangeInvalidatesMeasuredFlow)
{
    ASSERT_TRUE(RegisterTextTestFont());
    NeoWPF::UI::TextBlock textBlock(L"WWWWWWWWWW");
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(120.0, 240.0);
    const double defaultHeight = textBlock.GetDesiredHeight();

    textBlock.SetFontSize(32);
    textBlock.Measure(120.0, 240.0);

    EXPECT_GT(textBlock.GetDesiredHeight(), defaultHeight);
}

TEST(TextFlowTests, FontRegistrationAdvancesTheTextCacheGeneration)
{
    using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
    using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
    using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;
    ASSERT_GT(GetEmbeddedFallbackFontCount(), 0u);

    FontRecord font;
    ASSERT_TRUE(TryGetEmbeddedFallbackFont(0, font));
    ASSERT_NE(font.data, nullptr);
    ASSERT_GT(font.size, 0u);

    auto& database = NeoWPF::Rendering::Text::GetFontDatabase();
    const auto previousGeneration = database.Generation();
    ASSERT_TRUE(NeoWPF::Rendering::Text::RegisterFontMemory(
        font.data, font.size, L"NeoWPF Generation Test Font"));

    EXPECT_GT(database.Generation(), previousGeneration);
}

}