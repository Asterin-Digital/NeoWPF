#include <gtest/gtest.h>

#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/UI/Localization/LocaleString.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>
#include <NeoWPF/UI/Localization/LocaleExpression.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Localization/LocalizationManager.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace {

using NeoWPF::UI::Localization::Culture;
using NeoWPF::UI::Localization::LocaleString;
using NeoWPF::UI::Localization::LocaleTable;
using NeoWPF::UI::Localization::LocaleExpression;
using NeoWPF::UI::Localization::LocaleOperations;
using NeoWPF::UI::Localization::LocalizationManager;
using NeoWPF::UI::Localization::MissingKeyPolicy;

class LocalizationManagerTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        manager().Clear();
    }

    void TearDown() override
    {
        manager().Clear();
    }

    static LocalizationManager& manager()
    {
        return LocalizationManager::Get();
    }
};

TEST(CultureTests, NormalizesLanguageAndRegion)
{
    const Culture culture(L"en_us");

    ASSERT_TRUE(culture.IsValid());
    EXPECT_EQ(culture.GetName(), L"en-US");
    EXPECT_EQ(culture.GetLanguage(), L"en");
    EXPECT_EQ(culture.GetRegion(), L"US");
}

TEST(CultureTests, BuildsRegionalFallbackChain)
{
    const auto chain = Culture(L"ro-RO").GetFallbackChain();

    ASSERT_EQ(chain.size(), 2u);
    EXPECT_EQ(chain[0].GetName(), L"ro-RO");
    EXPECT_EQ(chain[1].GetName(), L"ro");
}

TEST(CultureTests, LanguageOnlyCultureHasNoDuplicateFallback)
{
    const auto chain = Culture(L"de").GetFallbackChain();

    ASSERT_EQ(chain.size(), 1u);
    EXPECT_EQ(chain[0].GetName(), L"de");
}

TEST(CultureTests, RejectsInvalidCultureNames)
{
    EXPECT_FALSE(Culture().IsValid());
    EXPECT_FALSE(Culture(L"").IsValid());
    EXPECT_FALSE(Culture(L"e").IsValid());
    EXPECT_FALSE(Culture(L"en-").IsValid());
    EXPECT_FALSE(Culture(L"en-US-extra").IsValid());
    EXPECT_FALSE(Culture(L"en-_").IsValid());
}

TEST(LocaleStringTests, StoresKeyAndValue)
{
    const LocaleString value(L"Greeting", L"Hello");

    EXPECT_EQ(value.GetKey(), L"Greeting");
    EXPECT_EQ(value.GetValue(), L"Hello");
}

TEST(LocaleTableTests, RejectsInvalidCulture)
{
    LocaleTable table{Culture()};

    EXPECT_FALSE(table.IsValid());
    EXPECT_FALSE(table.Add(L"Greeting", L"Hello"));
}

TEST(LocaleTableTests, AddsAndReadsEntries)
{
    LocaleTable table(L"en-US");
    std::wstring value;

    ASSERT_TRUE(table.Add(L"Greeting", L"Hello"));
    EXPECT_TRUE(table.TryGet(L"Greeting", value));
    EXPECT_EQ(value, L"Hello");
    EXPECT_EQ(table.GetStrings().size(), 1u);
}

TEST(LocaleTableTests, RejectsEmptyAndDuplicateKeys)
{
    LocaleTable table(L"en-US");

    EXPECT_FALSE(table.Add(L"", L"Empty key"));
    EXPECT_TRUE(table.Add(L"Greeting", L"Hello"));
    EXPECT_FALSE(table.Add(LocaleString(L"Greeting", L"Bonjour")));

    std::wstring value;
    ASSERT_TRUE(table.TryGet(L"Greeting", value));
    EXPECT_EQ(value, L"Hello");
}

TEST(LocaleTableTests, MissingEntryDoesNotChangeOutput)
{
    LocaleTable table(L"en-US");
    std::wstring value = L"unchanged";

    EXPECT_FALSE(table.TryGet(L"Missing", value));
    EXPECT_EQ(value, L"unchanged");
}

TEST_F(LocalizationManagerTests, RegistersAndResolvesExactLocale)
{
    LocaleTable table(L"en-US");
    ASSERT_TRUE(table.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(table));
    ASSERT_TRUE(manager().SetActiveLocale(L"en-us"));

    std::wstring value;
    EXPECT_TRUE(manager().TryResolve(L"Greeting", value));
    EXPECT_EQ(value, L"Hello");
    EXPECT_EQ(manager().Resolve(L"Greeting"), L"Hello");
}

TEST_F(LocalizationManagerTests, ResolvesLanguageOnlyFallbackBeforeConfiguredFallback)
{
    LocaleTable languageTable(L"ro");
    LocaleTable fallbackTable(L"en-US");
    ASSERT_TRUE(languageTable.Add(L"Greeting", L"Salut"));
    ASSERT_TRUE(fallbackTable.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(languageTable));
    ASSERT_TRUE(manager().RegisterTable(fallbackTable));
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));
    ASSERT_TRUE(manager().SetFallbackLocale(L"en-US"));

    EXPECT_EQ(manager().Resolve(L"Greeting"), L"Salut");
}

TEST_F(LocalizationManagerTests, ResolvesConfiguredFallbackWhenActiveLocaleMisses)
{
    LocaleTable table(L"en-US");
    ASSERT_TRUE(table.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(table));
    ASSERT_TRUE(manager().SetActiveLocale(L"fr-FR"));
    ASSERT_TRUE(manager().SetFallbackLocale(L"en-US"));

    EXPECT_EQ(manager().Resolve(L"Greeting"), L"Hello");
}

TEST_F(LocalizationManagerTests, ExactActiveLocaleWinsOverLanguageAndFallback)
{
    LocaleTable exact(L"ro-RO");
    LocaleTable language(L"ro");
    LocaleTable fallback(L"en-US");
    ASSERT_TRUE(exact.Add(L"Greeting", L"Buna"));
    ASSERT_TRUE(language.Add(L"Greeting", L"Salut"));
    ASSERT_TRUE(fallback.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(exact));
    ASSERT_TRUE(manager().RegisterTable(language));
    ASSERT_TRUE(manager().RegisterTable(fallback));
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));
    ASSERT_TRUE(manager().SetFallbackLocale(L"en-US"));

    EXPECT_EQ(manager().Resolve(L"Greeting"), L"Buna");
}

TEST_F(LocalizationManagerTests, MissingKeyUsesMarkerByDefault)
{
    ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));

    std::wstring value;
    EXPECT_FALSE(manager().TryResolve(L"Missing.Title", value));
    EXPECT_EQ(manager().Resolve(L"Missing.Title"), L"[[Missing.Title]]");
    EXPECT_EQ(manager().GetMissingKeyPolicy(), MissingKeyPolicy::Marker);
}

TEST_F(LocalizationManagerTests, MissingKeyPoliciesAreApplied)
{
    ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));

    manager().SetMissingKeyPolicy(MissingKeyPolicy::Empty);
    EXPECT_TRUE(manager().Resolve(L"Missing").empty());

    manager().SetMissingKeyPolicy(MissingKeyPolicy::Key);
    EXPECT_EQ(manager().Resolve(L"Missing"), L"Missing");
}

TEST_F(LocalizationManagerTests, RegisterRejectsDuplicateCultureAndReplaceUpdatesIt)
{
    LocaleTable first(L"en-US");
    LocaleTable second(L"en_us");
    ASSERT_TRUE(first.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(second.Add(L"Greeting", L"Hi"));
    ASSERT_TRUE(manager().RegisterTable(first));
    EXPECT_FALSE(manager().RegisterTable(second));
    EXPECT_TRUE(manager().ReplaceTable(second));
    ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));

    EXPECT_EQ(manager().Resolve(L"Greeting"), L"Hi");
}

TEST_F(LocalizationManagerTests, RemoveTableStopsResolution)
{
    LocaleTable table(L"en-US");
    ASSERT_TRUE(table.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(table));
    ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));
    ASSERT_TRUE(manager().RemoveTable(Culture(L"en_us")));

    EXPECT_EQ(manager().Resolve(L"Greeting"), L"[[Greeting]]");
    EXPECT_FALSE(manager().RemoveTable(Culture(L"en-US")));
}

TEST_F(LocalizationManagerTests, RejectsInvalidLocaleConfiguration)
{
    std::wstring value;

    EXPECT_FALSE(manager().RegisterTable(LocaleTable(Culture())));
    EXPECT_FALSE(manager().SetActiveLocale(L"invalid-"));
    EXPECT_FALSE(manager().SetFallbackLocale(L"x"));
    EXPECT_FALSE(manager().RemoveTable(Culture()));
    EXPECT_FALSE(manager().TryResolve(L"", value));
}

TEST_F(LocalizationManagerTests, ClearResetsTablesConfigurationAndPolicy)
{
    LocaleTable table(L"en-US");
    ASSERT_TRUE(table.Add(L"Greeting", L"Hello"));
    ASSERT_TRUE(manager().RegisterTable(table));
    ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));
    ASSERT_TRUE(manager().SetFallbackLocale(L"en-US"));
    manager().SetMissingKeyPolicy(MissingKeyPolicy::Key);

    manager().Clear();

    EXPECT_FALSE(manager().GetActiveLocale().IsValid());
    EXPECT_FALSE(manager().GetFallbackLocale().IsValid());
    EXPECT_EQ(manager().GetMissingKeyPolicy(), MissingKeyPolicy::Marker);
    EXPECT_EQ(manager().Resolve(L"Greeting"), L"[[Greeting]]");
}

class LocaleExpressionTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        manager().Clear();

        LocaleTable english(L"en-US");
        LocaleTable romanian(L"ro-RO");
        ASSERT_TRUE(english.Add(L"Greeting", L"Hello"));
        ASSERT_TRUE(romanian.Add(L"Greeting", L"Salut"));
        ASSERT_TRUE(manager().RegisterTable(english));
        ASSERT_TRUE(manager().RegisterTable(romanian));
        ASSERT_TRUE(manager().SetActiveLocale(L"en-US"));
    }

    void TearDown() override
    {
        manager().Clear();
    }

    static LocalizationManager& manager()
    {
        return LocalizationManager::Get();
    }
};

TEST_F(LocaleExpressionTests, ResolvesImmediatelyAndReportsExpressionSource)
{
    NeoWPF::UI::TextBlock textBlock;

    const auto expression = LocaleOperations::SetLocale(
        textBlock,
        NeoWPF::UI::TextBlock::TextProperty(),
        L"Greeting");

    ASSERT_NE(expression, nullptr);
    EXPECT_EQ(expression->GetKey(), L"Greeting");
    EXPECT_EQ(textBlock.GetText(), L"Hello");
    EXPECT_EQ(textBlock.GetValueSource(NeoWPF::UI::TextBlock::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Expression);
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()).get(),
              expression.get());
}

TEST_F(LocaleExpressionTests, RefreshesWhenActiveLocaleChanges)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBlock.GetText(), L"Salut");
}

TEST_F(LocaleExpressionTests, ResolvesImmediatelyForTextBox)
{
    NeoWPF::UI::TextBox textBox;

    const auto expression = LocaleOperations::SetLocale(
        textBox,
        NeoWPF::UI::TextBox::TextProperty(),
        L"Greeting");

    ASSERT_NE(expression, nullptr);
    EXPECT_EQ(textBox.GetText(), L"Hello");
    EXPECT_EQ(textBox.GetValueSource(NeoWPF::UI::TextBox::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Expression);
}

TEST_F(LocaleExpressionTests, RefreshesTextBoxWhenActiveLocaleChanges)
{
    NeoWPF::UI::TextBox textBox;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBox,
                  NeoWPF::UI::TextBox::TextProperty(),
                  L"Greeting"), nullptr);

    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBox.GetText(), L"Salut");
}

TEST_F(LocaleExpressionTests, DirectTextBoxWriteDetachesLocaleExpression)
{
    NeoWPF::UI::TextBox textBox;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBox,
                  NeoWPF::UI::TextBox::TextProperty(),
                  L"Greeting"), nullptr);

    textBox.SetText(L"Custom");
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBox.GetText(), L"Custom");
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBox,
                  NeoWPF::UI::TextBox::TextProperty()), nullptr);
    EXPECT_EQ(textBox.GetValueSource(NeoWPF::UI::TextBox::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Local);
}

TEST_F(LocaleExpressionTests, DirectTextWriteDetachesLocaleExpression)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    textBlock.SetText(L"Custom");
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBlock.GetText(), L"Custom");
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()), nullptr);
    EXPECT_EQ(textBlock.GetValueSource(NeoWPF::UI::TextBlock::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Local);
}

TEST_F(LocaleExpressionTests, DirectDependencyPropertyWriteDetachesLocaleExpression)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    textBlock.SetValue(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Custom"));
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBlock.GetText(), L"Custom");
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()), nullptr);
}

TEST_F(LocaleExpressionTests, ReplacingLocaleDetachesPreviousExpression)
{
    LocaleTable english(L"en-US");
    ASSERT_TRUE(english.Add(L"Farewell", L"Goodbye"));
    ASSERT_TRUE(manager().ReplaceTable(english));

    NeoWPF::UI::TextBlock textBlock;
    const auto first = LocaleOperations::SetLocale(
        textBlock,
        NeoWPF::UI::TextBlock::TextProperty(),
        L"Greeting");
    const auto second = LocaleOperations::SetLocale(
        textBlock,
        NeoWPF::UI::TextBlock::TextProperty(),
        L"Farewell");

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(textBlock.GetText(), L"Goodbye");
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()).get(),
              second.get());

    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));
    EXPECT_EQ(textBlock.GetText(), L"[[Farewell]]");
}

TEST_F(LocaleExpressionTests, ClearLocaleRemovesExpressionValue)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    LocaleOperations::ClearLocale(textBlock, NeoWPF::UI::TextBlock::TextProperty());

    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()), nullptr);
    EXPECT_EQ(textBlock.GetText(), L"");
    EXPECT_EQ(textBlock.GetValueSource(NeoWPF::UI::TextBlock::TextProperty()),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
}

TEST_F(LocaleExpressionTests, ExplicitDetachStopsFutureLocaleUpdates)
{
    NeoWPF::UI::TextBlock textBlock;
    const auto expression = LocaleOperations::SetLocale(
        textBlock,
        NeoWPF::UI::TextBlock::TextProperty(),
        L"Greeting");
    ASSERT_NE(expression, nullptr);

    expression->Detach();
    ASSERT_TRUE(manager().SetActiveLocale(L"ro-RO"));

    EXPECT_EQ(textBlock.GetText(), L"Hello");
    EXPECT_EQ(LocaleOperations::GetLocaleExpression(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty()).get(),
              expression.get());
}

TEST_F(LocaleExpressionTests, ReplacingTableRefreshesAttachedExpressions)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    LocaleTable replacement(L"en-US");
    ASSERT_TRUE(replacement.Add(L"Greeting", L"Welcome"));
    ASSERT_TRUE(manager().ReplaceTable(replacement));

    EXPECT_EQ(textBlock.GetText(), L"Welcome");
}

TEST_F(LocaleExpressionTests, RemovingTableRefreshesAttachedExpressions)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    ASSERT_TRUE(manager().RemoveTable(Culture(L"en-US")));

    EXPECT_EQ(textBlock.GetText(), L"[[Greeting]]");
}

TEST_F(LocaleExpressionTests, ClearingManagerRefreshesAttachedExpressions)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Greeting"), nullptr);

    manager().Clear();

    EXPECT_EQ(textBlock.GetText(), L"[[Greeting]]");
}

TEST_F(LocaleExpressionTests, MissingKeyPolicyChangesRefreshAttachedExpressions)
{
    NeoWPF::UI::TextBlock textBlock;
    ASSERT_NE(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L"Missing"), nullptr);
    EXPECT_EQ(textBlock.GetText(), L"[[Missing]]");

    manager().SetMissingKeyPolicy(MissingKeyPolicy::Key);

    EXPECT_EQ(textBlock.GetText(), L"Missing");
}

TEST_F(LocaleExpressionTests, RejectsEmptyLocaleKey)
{
    NeoWPF::UI::TextBlock textBlock;

    EXPECT_EQ(LocaleOperations::SetLocale(
                  textBlock,
                  NeoWPF::UI::TextBlock::TextProperty(),
                  L""), nullptr);
    EXPECT_EQ(textBlock.GetText(), L"");
}

}
