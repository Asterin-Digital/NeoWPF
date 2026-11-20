#include <gtest/gtest.h>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Layout/Canvas/Canvas.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/UniformGrid/UniformGrid.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>

#include <memory>
#include <vector>

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

void EnsureResponsiveTestPropertiesRegistered()
{
    static const bool registered = [] {
        NeoWPF::UI::Properties::RegisterPanelProperties();
        NeoWPF::UI::Control::PaddingProperty();
        return true;
    }();
    (void)registered;
}

TEST(PanelTests, MeasuresAndArrangesMixedControls)
{
    NeoWPF::UI::Panel panel;
    panel.SetPadding(NeoWPF::UI::Thickness(2, 3, 4, 5));

    auto button = std::make_shared<NeoWPF::UI::Button>(L"Play");
    button->SetWidth(80.0);
    button->SetHeight(32.0);
    auto label = std::make_shared<NeoWPF::UI::TextBlock>(L"Status");
    auto checkbox = std::make_shared<NeoWPF::UI::Checkbox>();
    checkbox->SetText(L"Sound");

    panel.AddChild(button);
    panel.AddChild(label);
    panel.AddChild(checkbox);

    panel.Measure(300.0, 200.0);
    panel.Arrange(300.0, 200.0);

    EXPECT_GT(panel.GetDesiredWidth(), 0.0);
    EXPECT_GT(panel.GetDesiredHeight(), 0.0);
    EXPECT_DOUBLE_EQ(button->GetFinalWidth(), 80.0);
    EXPECT_DOUBLE_EQ(button->GetFinalHeight(), 32.0);
    EXPECT_EQ(panel.GetVisualChildren().size(), 3u);
    EXPECT_EQ(button->GetParent(), &panel);
    EXPECT_EQ(label->GetParent(), &panel);
    EXPECT_EQ(checkbox->GetParent(), &panel);
}

TEST(PanelTests, SkipsInvisibleChildrenButKeepsVisibleLayoutStable)
{
    NeoWPF::UI::Panel panel;
    auto visible = std::make_shared<FixedElement>(40.0, 20.0);
    auto hidden = std::make_shared<FixedElement>(500.0, 500.0);
    hidden->SetVisibility(false);
    panel.AddChild(visible);
    panel.AddChild(hidden);

    panel.Measure(100.0, 100.0);
    panel.Arrange(100.0, 100.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 100.0);
    EXPECT_DOUBLE_EQ(panel.GetDesiredHeight(), 100.0);
    EXPECT_DOUBLE_EQ(visible->GetFinalWidth(), 40.0);
    EXPECT_DOUBLE_EQ(visible->GetFinalHeight(), 20.0);
}

TEST(PanelTests, StackPanelSupportsHorizontalMixedChildrenAndSpacing)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    panel.SetSpacing(7.0);

    auto first = std::make_shared<FixedElement>(20.0, 10.0);
    auto second = std::make_shared<NeoWPF::UI::Button>(L"Next");
    second->SetWidth(60.0);
    second->SetHeight(24.0);
    auto third = std::make_shared<FixedElement>(30.0, 12.0);
    panel.AddChild(first);
    panel.AddChild(second);
    panel.AddChild(third);

    panel.Measure(200.0, 80.0);
    panel.Arrange(200.0, 80.0);

    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), 20.0 + 7.0 + 60.0 + 7.0 + 30.0);
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetX(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetX(), 27.0);
    EXPECT_DOUBLE_EQ(third->GetLayoutOffsetX(), 94.0);
}

TEST(PanelTests, NestedPanelsPreserveChildOrderAndOffsets)
{
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    root->SetSpacing(4.0);
    auto top = std::make_shared<NeoWPF::UI::TextBlock>(L"Header");
    auto nested = std::make_shared<NeoWPF::UI::StackPanel>();
    nested->SetSpacing(3.0);
    auto first = std::make_shared<FixedElement>(50.0, 10.0);
    auto second = std::make_shared<FixedElement>(70.0, 20.0);
    nested->AddChild(first);
    nested->AddChild(second);
    auto bottom = std::make_shared<NeoWPF::UI::Checkbox>();
    bottom->SetText(L"Footer");

    root->AddChild(top);
    root->AddChild(nested);
    root->AddChild(bottom);
    root->Measure(300.0, 300.0);
    root->Arrange(300.0, 300.0);

    EXPECT_EQ(root->GetVisualChildren()[0], top.get());
    EXPECT_EQ(root->GetVisualChildren()[1], nested.get());
    EXPECT_EQ(root->GetVisualChildren()[2], bottom.get());
    EXPECT_DOUBLE_EQ(first->GetLayoutOffsetY(), 0.0);
    EXPECT_DOUBLE_EQ(second->GetLayoutOffsetY(), 13.0);
    EXPECT_GT(bottom->GetLayoutOffsetY(), nested->GetLayoutOffsetY());
}

TEST(PanelTests, DeeplyNestedStackPanelsMeasureAndArrange)
{
    auto root = std::make_shared<NeoWPF::UI::StackPanel>();
    auto current = root;
    constexpr int depth = 64;
    for (int index = 0; index < depth; ++index) {
        auto child = std::make_shared<NeoWPF::UI::StackPanel>();
        current->AddChild(child);
        current = child;
    }
    auto leaf = std::make_shared<FixedElement>(48.0, 20.0);
    current->AddChild(leaf);

    root->Measure(400.0, 400.0);
    root->Arrange(400.0, 400.0);

    EXPECT_DOUBLE_EQ(leaf->GetFinalWidth(), 48.0);
    EXPECT_DOUBLE_EQ(leaf->GetFinalHeight(), 20.0);
    EXPECT_EQ(root->GetVisualDescendants().size(), static_cast<std::size_t>(depth + 1));
}

TEST(PanelTests, LargePanelPreservesAllChildrenAndRenders)
{
    NeoWPF::UI::StackPanel panel;
    panel.SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    panel.SetSpacing(1.0);

    constexpr int childCount = 256;
    for (int index = 0; index < childCount; ++index) {
        auto child = std::make_shared<FixedElement>(2.0, 3.0);
        panel.AddChild(child);
    }

    panel.Measure(1000.0, 20.0);
    panel.Arrange(1000.0, 20.0);
    NeoWPF::Rendering::CommandList commands;
    panel.BuildRenderCommands(commands);

    EXPECT_EQ(panel.GetVisualChildren().size(), static_cast<std::size_t>(childCount));
    EXPECT_DOUBLE_EQ(panel.GetDesiredWidth(), childCount * 2.0 + (childCount - 1) * 1.0);
}

TEST(PanelTests, ResponsiveRulesUseOrderedInclusiveMinimumAndExclusiveMaximumBounds)
{
    EnsureResponsiveTestPropertiesRegistered();
    NeoWPF::UI::StackPanel panel;
    const auto* const orientationProperty =
        panel.FindDependencyProperty("Orientation");
    ASSERT_NE(orientationProperty, nullptr);

    auto wideRule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    wideRule->SetMinWidth(100.0);
    wideRule->SetMaxWidth(200.0);
    wideRule->SetMinHeight(50.0);
    wideRule->SetMaxHeight(150.0);
    wideRule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Orientation", static_cast<int>(NeoWPF::UI::Orientation::Horizontal)));

    auto overlappingRule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    overlappingRule->SetMinWidth(150.0);
    overlappingRule->SetMaxWidth(250.0);
    overlappingRule->SetMinHeight(50.0);
    overlappingRule->SetMaxHeight(150.0);
    overlappingRule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Orientation", static_cast<int>(NeoWPF::UI::Orientation::Vertical)));

    panel.AddResponsiveRule(wideRule);
    panel.AddResponsiveRule(overlappingRule);

    panel.Measure(100.0, 50.0);
    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Horizontal);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Responsive);

    panel.Measure(150.0, 50.0);
    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Vertical);

    panel.Measure(200.0, 149.999);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Responsive);

    panel.Measure(250.0, 149.999);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Vertical);

    panel.Measure(100.0, 150.0);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
}

TEST(PanelTests, ResponsiveRulesRespectLocalValuesAndRestoreThemAfterRemoval)
{
    EnsureResponsiveTestPropertiesRegistered();
    NeoWPF::UI::StackPanel panel;
    const auto* const orientationProperty =
        panel.FindDependencyProperty("Orientation");
    ASSERT_NE(orientationProperty, nullptr);

    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    rule->SetMinWidth(100.0);
    rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Orientation", static_cast<int>(NeoWPF::UI::Orientation::Vertical)));
    panel.AddResponsiveRule(rule);

    panel.Measure(120.0, 80.0);
    panel.SetValue(
        *orientationProperty,
        static_cast<int>(NeoWPF::UI::Orientation::Horizontal));
    panel.Measure(120.0, 80.0);
    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Horizontal);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Local);

    panel.ClearValue(orientationProperty);
    EXPECT_EQ(panel.GetOrientation(), NeoWPF::UI::Orientation::Vertical);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Responsive);

    panel.Measure(99.0, 80.0);
    EXPECT_EQ(panel.GetValueSource(orientationProperty),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
}

TEST(PanelTests, ResponsiveRulesAreEvaluatedByEveryBuiltInPanel)
{
    EnsureResponsiveTestPropertiesRegistered();
    NeoWPF::UI::Control::PaddingProperty();
    NeoWPF::UI::Panel panel;
    NeoWPF::UI::StackPanel stackPanel;
    NeoWPF::UI::Canvas canvas;
    NeoWPF::UI::Grid grid;
    NeoWPF::UI::UniformGrid uniformGrid;
    NeoWPF::UI::Panel* const panels[] = {
        &panel, &stackPanel, &canvas, &grid, &uniformGrid};

    for (auto* const current : panels) {
        auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
        rule->SetMinWidth(80.0);
        rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
            "Padding", NeoWPF::UI::Thickness(12.0, 0.0, 0.0, 0.0)));
        current->AddResponsiveRule(std::move(rule));

        current->Measure(100.0, 60.0);
        EXPECT_DOUBLE_EQ(current->GetPadding().Left, 12.0);

        current->Measure(79.0, 60.0);
        EXPECT_DOUBLE_EQ(current->GetPadding().Left, 0.0);
    }
}

TEST(PanelTests, ResponsiveRuleChangesInvalidateAttachedPanel)
{
    EnsureResponsiveTestPropertiesRegistered();

    class InvalidationTrackingStackPanel final : public NeoWPF::UI::StackPanel
    {
    public:
        void InvalidateMeasure() override
        {
            ++invalidationCount;
            StackPanel::InvalidateMeasure();
        }

        int invalidationCount = 0;
    };

    InvalidationTrackingStackPanel panel;
    InvalidationTrackingStackPanel otherPanel;
    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    panel.AddResponsiveRule(rule);
    otherPanel.AddResponsiveRule(rule);

    int observedInvalidations = panel.invalidationCount;
    int observedOtherInvalidations = otherPanel.invalidationCount;
    rule->SetMinWidth(80.0);
    EXPECT_GT(panel.invalidationCount, observedInvalidations);
    EXPECT_GT(otherPanel.invalidationCount, observedOtherInvalidations);

    observedInvalidations = panel.invalidationCount;
    observedOtherInvalidations = otherPanel.invalidationCount;
    rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Padding", NeoWPF::UI::Thickness(12.0, 0.0, 0.0, 0.0)));
    EXPECT_GT(panel.invalidationCount, observedInvalidations);
    EXPECT_GT(otherPanel.invalidationCount, observedOtherInvalidations);
}

TEST(PanelTests, ResponsiveCollectionsAreStoredInDependencyProperties)
{
    EnsureResponsiveTestPropertiesRegistered();
    NeoWPF::UI::StackPanel panel;
    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    auto setter = std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Padding", NeoWPF::UI::Thickness(12.0, 0.0, 0.0, 0.0));

    rule->AddSetter(setter);
    panel.AddResponsiveRule(rule);

    const auto& panelRules =
        panel.GetValue<std::vector<std::shared_ptr<NeoWPF::UI::ResponsiveRule>>>(
            NeoWPF::UI::Panel::ResponsiveRulesProperty());
    ASSERT_EQ(panelRules.size(), 1u);
    EXPECT_EQ(panelRules.front(), rule);

    const auto& ruleSetters =
        rule->GetValue<std::vector<std::shared_ptr<NeoWPF::UI::Styling::Triggers::Setter>>>(
            NeoWPF::UI::ResponsiveRule::SettersProperty());
    ASSERT_EQ(ruleSetters.size(), 1u);
    EXPECT_EQ(ruleSetters.front(), setter);

    EXPECT_EQ(panel.GetResponsiveRules().size(), 1u);
    EXPECT_EQ(rule->GetSetters().size(), 1u);

    auto replacementSetter =
        std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
            "Padding", NeoWPF::UI::Thickness(20.0, 0.0, 0.0, 0.0));
    rule->SetValue(
        NeoWPF::UI::ResponsiveRule::SettersProperty(),
        std::vector<std::shared_ptr<NeoWPF::UI::Styling::Triggers::Setter>>{
            replacementSetter});
    ASSERT_EQ(rule->GetSetters().size(), 1u);
    EXPECT_EQ(rule->GetSetters().front(), replacementSetter);

    auto replacementRule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    panel.SetValue(
        NeoWPF::UI::Panel::ResponsiveRulesProperty(),
        std::vector<std::shared_ptr<NeoWPF::UI::ResponsiveRule>>{
            replacementRule});
    ASSERT_EQ(panel.GetResponsiveRules().size(), 1u);
    EXPECT_EQ(panel.GetResponsiveRules().front(), replacementRule);
}

TEST(PanelTests, ChangingAttachedSetterInvalidatesAttachedPanels)
{
    EnsureResponsiveTestPropertiesRegistered();

    class InvalidationTrackingStackPanel final : public NeoWPF::UI::StackPanel
    {
    public:
        void InvalidateMeasure() override
        {
            ++invalidationCount;
            StackPanel::InvalidateMeasure();
        }

        int invalidationCount = 0;
    };

    InvalidationTrackingStackPanel panel;
    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    rule->SetMinWidth(80.0);
    auto setter = std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Padding", NeoWPF::UI::Thickness(12.0, 0.0, 0.0, 0.0));
    rule->AddSetter(setter);
    panel.AddResponsiveRule(rule);

    panel.Measure(100.0, 60.0);
    EXPECT_DOUBLE_EQ(panel.GetPadding().Left, 12.0);

    const int observedInvalidations = panel.invalidationCount;
    setter->SetValue(NeoWPF::UI::Thickness(24.0, 0.0, 0.0, 0.0));
    EXPECT_GT(panel.invalidationCount, observedInvalidations);

    panel.Measure(100.0, 60.0);
    EXPECT_DOUBLE_EQ(panel.GetPadding().Left, 24.0);

    setter->SetProperty("UnknownPaddingProperty");
    panel.Measure(100.0, 60.0);
    EXPECT_DOUBLE_EQ(panel.GetPadding().Left, 0.0);
}

} 
