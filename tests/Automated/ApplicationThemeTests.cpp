#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace {

using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::Core::ApplicationThemes::ThemeBorderThickness;
using NeoWPF::Core::ApplicationThemes::ThemeCornerRadius;
using NeoWPF::Core::ApplicationThemes::ThemeDefinition;
using NeoWPF::Core::ApplicationThemes::ThemeFontSize;
using NeoWPF::Core::ApplicationThemes::ThemeManager;
using NeoWPF::Core::ApplicationThemes::ThemeMetric;
using NeoWPF::Core::ApplicationThemes::ThemePadding;

std::shared_ptr<NeoWPF::UI::Brush> MakeSolidBrush(std::uint32_t argb)
{
    return std::make_shared<NeoWPF::UI::SolidColorBrush>(argb);
}

std::uint32_t GetSolidBrushArgb(const std::shared_ptr<NeoWPF::UI::Brush>& brush)
{
    const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush);
    return solidBrush ? solidBrush->GetArgb() : 0u;
}

class ApplicationThemeTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        ThemeManager::Instance().Clear();
    }

    void TearDown() override
    {
        ThemeManager::Instance().Clear();
    }

    static ThemeDefinition MakeTheme(const std::string& name, std::uint32_t accent,
                                   std::uint32_t background = 0xFF101010u)
    {
        ThemeDefinition theme;
        theme.Name = name;
        theme.Palette[PaletteBrush::Background] = MakeSolidBrush(background);
        theme.Palette[PaletteBrush::Surface] = MakeSolidBrush(0xFF202020u);
        theme.Palette[PaletteBrush::Foreground] = MakeSolidBrush(0xFFE0E0E0u);
        theme.Palette[PaletteBrush::MutedForeground] = MakeSolidBrush(0xFF808080u);
        theme.Palette[PaletteBrush::Border] = MakeSolidBrush(0xFF606060u);
        theme.Palette[PaletteBrush::Accent] = MakeSolidBrush(accent);
        theme.Palette[PaletteBrush::AccentHover] = MakeSolidBrush(0xFF404040u);
        theme.Palette[PaletteBrush::AccentPressed] = MakeSolidBrush(0xFF303030u);
        theme.Palette[PaletteBrush::DisabledSurface] = MakeSolidBrush(0xFF181818u);
        theme.Palette[PaletteBrush::Focus] = MakeSolidBrush(0xFF909090u);
        theme.Palette[PaletteBrush::Track] = MakeSolidBrush(0xFF505050u);
        theme.Palette[PaletteBrush::Thumb] = MakeSolidBrush(0xFF707070u);

        theme.FontSizes[ThemeFontSize::Control] = 15;
        theme.Paddings[ThemePadding::Button] = NeoWPF::UI::Thickness(8);
        theme.Paddings[ThemePadding::TextBlock] = NeoWPF::UI::Thickness(2);
        theme.Paddings[ThemePadding::TextBox] = NeoWPF::UI::Thickness(6, 4, 6, 4);
        theme.Paddings[ThemePadding::Checkbox] = NeoWPF::UI::Thickness(7);
        theme.Paddings[ThemePadding::Border] = NeoWPF::UI::Thickness(3);
        theme.Paddings[ThemePadding::ComboBox] = NeoWPF::UI::Thickness(9);
        theme.Paddings[ThemePadding::ComboBoxItem] = NeoWPF::UI::Thickness(5);
        theme.Paddings[ThemePadding::ListBox] = NeoWPF::UI::Thickness(4);
        theme.Paddings[ThemePadding::ListBoxItem] = NeoWPF::UI::Thickness(4);
        theme.Paddings[ThemePadding::ProgressBar] = NeoWPF::UI::Thickness(1);
        theme.Paddings[ThemePadding::RadioButton] = NeoWPF::UI::Thickness(8);
        theme.Paddings[ThemePadding::ScrollBar] = NeoWPF::UI::Thickness(0);
        theme.Paddings[ThemePadding::Slider] = NeoWPF::UI::Thickness(0);
        theme.BorderThicknesses[ThemeBorderThickness::Border] = NeoWPF::UI::Thickness(1);
        theme.Metrics[ThemeMetric::ScrollBarThumbSize] = 24.0;
        theme.Metrics[ThemeMetric::ProgressBarBorderThickness] = 1.5;
        theme.Metrics[ThemeMetric::SliderBorderThickness] = 2.0;
        theme.Metrics[ThemeMetric::SliderThumbLength] = 16.0;
        theme.Metrics[ThemeMetric::ScrollBarBorderThickness] = 1.0;
        theme.CornerRadii[ThemeCornerRadius::Control] = NeoWPF::UI::CornerRadius(4);
        theme.CornerRadii[ThemeCornerRadius::Checkbox] = NeoWPF::UI::CornerRadius(2);
        return theme;
    }
};

TEST_F(ApplicationThemeTests, RegistersAppliesAndRetrievesThemeValues)
{
    auto theme = MakeTheme("Test Theme", 0xFF112233u);
    auto& manager = ThemeManager::Instance();

    ASSERT_FALSE(manager.HasTheme(theme.Name));
    ASSERT_FALSE(manager.SetActiveTheme(theme.Name));

    manager.RegisterTheme(theme);
    ASSERT_TRUE(manager.HasTheme(theme.Name));
    ASSERT_TRUE(manager.SetActiveTheme(theme.Name));

    EXPECT_EQ(manager.GetActiveThemeName(), theme.Name);
    ASSERT_NE(manager.GetActiveTheme(), nullptr);
    EXPECT_EQ(manager.GetActiveTheme()->Name, theme.Name);

    std::shared_ptr<NeoWPF::UI::Brush> brush;
    ASSERT_TRUE(manager.TryGetPaletteBrush(PaletteBrush::Background, brush));
    EXPECT_EQ(GetSolidBrushArgb(brush), 0xFF101010u);
    EXPECT_EQ(GetSolidBrushArgb(manager.GetActiveTheme()->Palette.at(PaletteBrush::Accent)), 0xFF112233u);

    int fontSize = 0;
    ASSERT_TRUE(manager.TryGetFontSize(ThemeFontSize::Control, fontSize));
    EXPECT_EQ(fontSize, 15);

    NeoWPF::UI::Thickness padding;
    ASSERT_TRUE(manager.TryGetPadding(ThemePadding::Button, padding));
    EXPECT_EQ(padding, NeoWPF::UI::Thickness(8));

    NeoWPF::UI::Thickness border;
    ASSERT_TRUE(manager.TryGetBorderThickness(ThemeBorderThickness::Border, border));
    EXPECT_EQ(border, NeoWPF::UI::Thickness(1));

    double metric = 0.0;
    ASSERT_TRUE(manager.TryGetMetric(ThemeMetric::SliderThumbLength, metric));
    EXPECT_DOUBLE_EQ(metric, 16.0);

    NeoWPF::UI::CornerRadius radius;
    ASSERT_TRUE(manager.TryGetCornerRadius(ThemeCornerRadius::Checkbox, radius));
    EXPECT_EQ(radius, NeoWPF::UI::CornerRadius(2));

    EXPECT_TRUE(manager.ApplyTheme(theme));
    EXPECT_EQ(manager.GetActiveThemeName(), theme.Name);
    EXPECT_TRUE(manager.TryGetPaletteBrush(PaletteBrush::Accent, brush));
    EXPECT_EQ(GetSolidBrushArgb(brush), 0xFF112233u);
}

TEST_F(ApplicationThemeTests, RejectsInvalidThemeAndNotifiesOnApply)
{
    auto& manager = ThemeManager::Instance();
    ThemeDefinition invalidTheme;
    ThemeDefinition validTheme = MakeTheme("Valid Theme", 0xFF334455u);

    EXPECT_FALSE(manager.ApplyTheme(invalidTheme));
    EXPECT_FALSE(manager.SetActiveTheme("Missing Theme"));
    EXPECT_FALSE(manager.HasTheme(""));

    std::atomic<int> notifications{0};
    const auto subscription = manager.SubscribeThemeApplied([&notifications] {
        notifications.fetch_add(1, std::memory_order_relaxed);
    });
    ASSERT_NE(subscription, 0u);

    manager.RegisterTheme(validTheme);
    EXPECT_TRUE(manager.SetActiveTheme(validTheme.Name));
    EXPECT_EQ(notifications.load(std::memory_order_relaxed), 1);

    manager.UnsubscribeThemeApplied(subscription);
    EXPECT_TRUE(manager.SetActiveTheme(validTheme.Name));
    EXPECT_EQ(notifications.load(std::memory_order_relaxed), 1);
}

TEST_F(ApplicationThemeTests, RegistersAndAppliesThemeDefinitionsRepeatedly)
{
    auto& manager = ThemeManager::Instance();

    for (int index = 0; index < 500; ++index) {
        auto theme = MakeTheme("Stress Theme " + std::to_string(index),
                                static_cast<std::uint32_t>(0xFF010101u + index),
                                static_cast<std::uint32_t>(0xFF020202u + index));
        manager.RegisterTheme(theme);
        ASSERT_TRUE(manager.SetActiveTheme(theme.Name));
        EXPECT_EQ(manager.GetActiveThemeName(), theme.Name);

        std::shared_ptr<NeoWPF::UI::Brush> brush;
        ASSERT_TRUE(manager.TryGetPaletteBrush(PaletteBrush::Accent, brush));
        EXPECT_EQ(GetSolidBrushArgb(brush), static_cast<std::uint32_t>(0xFF010101u + index));
    }

    EXPECT_TRUE(manager.SetActiveTheme("Stress Theme 499"));
    EXPECT_EQ(manager.GetActiveThemeName(), "Stress Theme 499");
}

TEST_F(ApplicationThemeTests, AlternatesThemeStateAcrossRepeatedCycles)
{
    auto& manager = ThemeManager::Instance();
    ThemeDefinition first = MakeTheme("Cycle A", 0xFF111111u, 0xFF222222u);
    ThemeDefinition second = MakeTheme("Cycle B", 0xFF333333u, 0xFF444444u);
    manager.RegisterTheme(first);
    manager.RegisterTheme(second);

    for (int cycle = 0; cycle < 3000; ++cycle) {
        const bool useFirst = (cycle % 2) == 0;
        const auto& theme = useFirst ? first : second;

        ASSERT_TRUE(manager.SetActiveTheme(theme.Name));
        std::shared_ptr<NeoWPF::UI::Brush> brush;
        ASSERT_TRUE(manager.TryGetPaletteBrush(PaletteBrush::Background, brush));
        EXPECT_EQ(GetSolidBrushArgb(brush), useFirst ? 0xFF222222u : 0xFF444444u);

        EXPECT_EQ(GetSolidBrushArgb(manager.GetActiveTheme()->Palette.at(PaletteBrush::Accent)),
              useFirst ? 0xFF111111u : 0xFF333333u);

        EXPECT_TRUE(manager.ApplyTheme(theme));
        EXPECT_EQ(manager.GetActiveThemeName(), useFirst ? first.Name : second.Name);
    }
}

TEST_F(ApplicationThemeTests, ConcurrentThemeSwitchingRemainsConsistent)
{
    constexpr int kThemeCount = 64;
    constexpr std::uint32_t kAccentBase = 0xFF100000u;
    constexpr std::uint32_t kBackgroundBase = 0xFF200000u;

    auto& manager = ThemeManager::Instance();
    std::atomic<int> failure{0};
    std::atomic<int> notifications{0};
    const auto fail = [&failure](int code) {
        int expected = 0;
        failure.compare_exchange_strong(expected, code, std::memory_order_relaxed);
    };

    std::vector<std::uint32_t> validAccents;
    validAccents.reserve(kThemeCount);
    for (int index = 0; index < kThemeCount; ++index) {
        manager.RegisterTheme(MakeTheme("Theme " + std::to_string(index),
                                        kAccentBase + static_cast<std::uint32_t>(index),
                                        kBackgroundBase + static_cast<std::uint32_t>(index)));
        validAccents.push_back(kAccentBase + static_cast<std::uint32_t>(index));
    }

    const auto subscription = manager.SubscribeThemeApplied([&notifications] {
        notifications.fetch_add(1, std::memory_order_relaxed);
    });
    ASSERT_NE(subscription, 0u);

    std::vector<std::thread> workers;
    workers.reserve(4);
    for (int workerIndex = 0; workerIndex < 4; ++workerIndex) {
        workers.emplace_back([&, workerIndex] {
            for (int index = 0; index < 200; ++index) {
                const int target = (index + workerIndex) % kThemeCount;
                if (!manager.SetActiveTheme("Theme " + std::to_string(target))) {
                    fail(1);
                    return;
                }

                const auto snapshot = manager.GetActiveTheme();
                if (!snapshot || !manager.HasTheme(snapshot->Name)) {
                    fail(2);
                    return;
                }

                const auto accent = GetSolidBrushArgb(snapshot->Palette.at(PaletteBrush::Accent));
                const auto background = GetSolidBrushArgb(snapshot->Palette.at(PaletteBrush::Background));
                const auto snapshotIndex = accent - kAccentBase;
                if (accent < kAccentBase || snapshotIndex >= kThemeCount ||
                    background != kBackgroundBase + snapshotIndex ||
                    snapshot->Name != "Theme " + std::to_string(snapshotIndex)) {
                    fail(3);
                    return;
                }

                std::shared_ptr<NeoWPF::UI::Brush> brush;
                if (!manager.TryGetPaletteBrush(PaletteBrush::Accent, brush)) {
                    fail(4);
                    return;
                }
                if (std::find(validAccents.begin(), validAccents.end(), GetSolidBrushArgb(brush)) == validAccents.end()) {
                    fail(5);
                    return;
                }
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    manager.UnsubscribeThemeApplied(subscription);
    EXPECT_EQ(failure.load(std::memory_order_relaxed), 0)
        << "first failing check code (see legend in test)";
    EXPECT_GT(notifications.load(std::memory_order_relaxed), 0);
}

}
