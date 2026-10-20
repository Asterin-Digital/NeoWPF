#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Styling/Style.h>

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace {

namespace ApplicationResources = NeoWPF::Core::ApplicationResources;

class ApplicationResourcesTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ApplicationResources::Resources::Initialize();
    }

    void TearDown() override
    {
        ApplicationResources::Resources::Shutdown();
    }
};

TEST(ApplicationResourcesDictionaryTests, SetAndGetUpdatesValuesAndReportsMissingKeys)
{
    ApplicationResources::ResourceDictionary dictionary;
    std::any value = 42;

    EXPECT_FALSE(dictionary.Contains("Accent"));
    EXPECT_FALSE(dictionary.TryGet("Accent", value));
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::any_cast<int>(value), 42);
    EXPECT_FALSE(dictionary.Get("Accent").has_value());

    dictionary.Set("Accent", std::string("teal"));
    EXPECT_TRUE(dictionary.Contains("Accent"));
    ASSERT_TRUE(dictionary.TryGet("Accent", value));
    EXPECT_EQ(std::any_cast<std::string>(value), "teal");

    dictionary.Set("Accent", std::string("coral"));
    EXPECT_EQ(std::any_cast<std::string>(dictionary.Get("Accent")), "coral");
}

TEST(ApplicationResourcesDictionaryTests, SetManyAndMergeReplaceOnlyMatchingKeys)
{
    ApplicationResources::ResourceDictionary dictionary;
    dictionary.Set("Existing", 1);
    dictionary.Set("Preserved", 2);
    dictionary.SetMany({{"Existing", 3}, {"Added", 4}});

    ApplicationResources::ResourceDictionary merged;
    merged.Set("Existing", 5);
    merged.Set("Merged", 6);
    dictionary.Merge(merged);

    EXPECT_EQ(std::any_cast<int>(dictionary.Get("Existing")), 5);
    EXPECT_EQ(std::any_cast<int>(dictionary.Get("Preserved")), 2);
    EXPECT_EQ(std::any_cast<int>(dictionary.Get("Added")), 4);
    EXPECT_EQ(std::any_cast<int>(dictionary.Get("Merged")), 6);
}

TEST(ApplicationResourcesDictionaryTests, ImplicitStylesAreRetrievedByExactTypeAndCleared)
{
    ApplicationResources::ResourceDictionary dictionary;
    const std::type_index targetType(typeid(NeoWPF::UI::TextBlock));
    auto firstStyle = std::make_shared<NeoWPF::UI::Styling::Style>();
    auto secondStyle = std::make_shared<NeoWPF::UI::Styling::Style>();
    std::shared_ptr<NeoWPF::UI::Styling::Style> result;

    EXPECT_FALSE(dictionary.TryGetImplicitStyle(targetType, result));
    dictionary.SetImplicitStyle(targetType, firstStyle);
    ASSERT_TRUE(dictionary.TryGetImplicitStyle(targetType, result));
    EXPECT_EQ(result, firstStyle);

    dictionary.SetImplicitStyle(targetType, secondStyle);
    ASSERT_TRUE(dictionary.TryGetImplicitStyle(targetType, result));
    EXPECT_EQ(result, secondStyle);

    dictionary.Clear();
    EXPECT_FALSE(dictionary.Contains("unused"));
    EXPECT_FALSE(dictionary.TryGetImplicitStyle(targetType, result));
    EXPECT_EQ(result, secondStyle);
}

TEST_F(ApplicationResourcesTest, TryGetBrushAcceptsBrushValuesAndPreservesOutputOnFailure)
{
    auto storedBrush = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF123456u);
    std::shared_ptr<NeoWPF::UI::Brush> brush = storedBrush;
    ApplicationResources::Resources::SetResource("Accent", brush);

    std::shared_ptr<NeoWPF::UI::Brush> result;
    ASSERT_TRUE(ApplicationResources::Resources::TryGetBrush("Accent", result));
    EXPECT_EQ(result, storedBrush);

    auto previousBrush = std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFFABCDEFu);
    result = previousBrush;
    ApplicationResources::Resources::SetResource("NotABrush", 12);
    EXPECT_FALSE(ApplicationResources::Resources::TryGetBrush("NotABrush", result));
    EXPECT_EQ(result, previousBrush);
    EXPECT_FALSE(ApplicationResources::Resources::TryGetBrush("Missing", result));
    EXPECT_EQ(result, previousBrush);
}

TEST_F(ApplicationResourcesTest, FacadeSetsResourcesAndInitializeClearsResourcesAndStyles)
{
    ApplicationResources::Resources::SetResources({{"Count", 7}, {"Name", std::string("Neo")}});
    ApplicationResources::Resources::SetResource("Single", true);
    std::any value;

    ASSERT_TRUE(ApplicationResources::Resources::TryFindResource("Count", value));
    EXPECT_EQ(std::any_cast<int>(value), 7);
    EXPECT_EQ(std::any_cast<std::string>(ApplicationResources::Resources::FindResource("Name")), "Neo");
    ASSERT_TRUE(ApplicationResources::Resources::TryGetResource("Single", value));
    EXPECT_TRUE(std::any_cast<bool>(value));
    EXPECT_FALSE(ApplicationResources::Resources::TryFindResource("Missing", value));

    const std::type_index targetType(typeid(NeoWPF::UI::TextBlock));
    auto style = std::make_shared<NeoWPF::UI::Styling::Style>();
    ApplicationResources::Resources::SetImplicitStyle(targetType, style);
    std::shared_ptr<NeoWPF::UI::Styling::Style> resultStyle;
    ASSERT_TRUE(ApplicationResources::Resources::TryFindImplicitStyle(targetType, resultStyle));
    EXPECT_EQ(resultStyle, style);

    ApplicationResources::Resources::Initialize();
    EXPECT_FALSE(ApplicationResources::Resources::TryFindResource("Count", value));
    EXPECT_FALSE(ApplicationResources::Resources::TryFindImplicitStyle(targetType, resultStyle));
}

}