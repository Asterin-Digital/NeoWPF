#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/LayoutUtilities.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Responsive/ResponsiveRule.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Layout/LayoutEnvironment.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <limits>
#include <memory>

namespace {

class ResponsiveApplicationScope {
public:
    ResponsiveApplicationScope()
    {
        NeoWPF::Core::Application::Initialize();
    }

    ~ResponsiveApplicationScope()
    {
        NeoWPF::Core::Application::Shutdown();
    }
};

TEST(ResponsiveLayoutTests, TouchTargetResourceAndCoarseRuleApplyMinTargetSize)
{
    NeoWPF::Core::ApplicationResources::Resources::Initialize();
    double touchSize = 0.0;
    ASSERT_TRUE(NeoWPF::Core::ApplicationResources::Resources::TryGetDouble(
        "Touch.MinTargetSize", touchSize));
    EXPECT_DOUBLE_EQ(touchSize, 48.0);

    NeoWPF::UI::ResponsiveRule rule;
    rule.SetPointer(NeoWPF::UI::ResponsivePointer::Coarse);
    rule.AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "MinWidth", touchSize));
    rule.AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "MinHeight", touchSize));

    ASSERT_EQ(rule.GetSetters().size(), 2u);
}

TEST(ResponsiveLayoutTests, DependencyPropertyIsCanonicalSetterStorage)
{
    using Setter = NeoWPF::UI::Styling::Triggers::Setter;
    using Rule = NeoWPF::UI::ResponsiveRule;

    Rule rule;
    auto setter = std::make_shared<Setter>("Width", 240.0);
    rule.SetValue(Rule::SettersProperty(),
                  std::vector<std::shared_ptr<Setter>>{setter});

    EXPECT_EQ(rule.GetSetters(), (std::vector<std::shared_ptr<Setter>>{setter}));

    rule.ClearValue(Rule::SettersProperty());
    EXPECT_TRUE(rule.GetSetters().empty());
}

TEST(ResponsiveLayoutTests, PointerHoverAndKeyboardConditionsMatchWindowEnvironment)
{
    NeoWPF::UI::ResponsiveRule coarseRule;
    coarseRule.SetMinWidth(0.0);
    coarseRule.SetMaxWidth(NeoWPF::UI::Layout::Unbounded);
    coarseRule.SetMinHeight(0.0);
    coarseRule.SetMaxHeight(NeoWPF::UI::Layout::Unbounded);
    coarseRule.SetPointer(NeoWPF::UI::ResponsivePointer::Coarse);
    coarseRule.SetHover(NeoWPF::UI::ResponsiveHover::Off);
    coarseRule.SetKeyboard(NeoWPF::UI::ResponsiveKeyboard::Shown);

    EXPECT_TRUE(coarseRule.Matches(
        360.0,
        500.0,
        NeoWPF::UI::Layout::PointerPrecision::Coarse,
        false,
        true));
    EXPECT_FALSE(coarseRule.Matches(
        360.0,
        500.0,
        NeoWPF::UI::Layout::PointerPrecision::Fine,
        true,
        false));
}

TEST(ResponsiveLayoutTests, WindowScopeUsesWindowSizeAndInputEnvironment)
{
    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();

    NeoWPF::UI::Window window;
    NeoWPF::UI::Layout::LayoutEnvironment environment;
    environment.contentWidth = 1000.0;
    environment.contentHeight = 700.0;
    environment.pointer = NeoWPF::UI::Layout::PointerPrecision::Coarse;
    environment.hover = false;
    environment.keyboard.Bottom = 280.0;
    window.SetLayoutEnvironment(environment);

    auto host = std::make_shared<NeoWPF::UI::FrameworkElement>();
    window.SetContent(host);

    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    rule->SetScope(NeoWPF::UI::ResponsiveScope::Window);
    rule->SetMinWidth(800.0);
    rule->SetPointer(NeoWPF::UI::ResponsivePointer::Coarse);
    rule->SetHover(NeoWPF::UI::ResponsiveHover::Off);
    rule->SetKeyboard(NeoWPF::UI::ResponsiveKeyboard::Shown);
    rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Width", 240.0));
    host->AddResponsiveRule(rule);

    host->Measure(300.0, 400.0);
    EXPECT_DOUBLE_EQ(host->GetWidth(), 240.0);

    environment.pointer = NeoWPF::UI::Layout::PointerPrecision::Fine;
    environment.hover = true;
    environment.keyboard = {};
    window.SetLayoutEnvironment(environment);
    host->Measure(300.0, 400.0);

    EXPECT_DOUBLE_EQ(host->GetWidth(), 0.0);
}

TEST(ResponsiveLayoutTests, ContainerScopeUsesHostMeasureSize)
{
    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();

    NeoWPF::UI::Window window;
    NeoWPF::UI::Layout::LayoutEnvironment environment;
    environment.contentWidth = 1000.0;
    environment.contentHeight = 700.0;
    window.SetLayoutEnvironment(environment);

    auto host = std::make_shared<NeoWPF::UI::FrameworkElement>();
    window.SetContent(host);

    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    rule->SetMinWidth(800.0);
    rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Width", 240.0));
    host->AddResponsiveRule(rule);

    host->Measure(300.0, 400.0);
    EXPECT_DOUBLE_EQ(host->GetWidth(), 0.0);
}

TEST(ResponsiveLayoutTests, XamlRegistersResponsiveRuleAndSetterCollection)
{
    ResponsiveApplicationScope application;

    const auto roots = NeoWPF::UI::Markup::Xaml::ParseXaml(R"(
        <FrameworkElement>
            <FrameworkElement.ResponsiveRules>
                <ResponsiveRule MinWidth="320" Scope="Window">
                    <Setter Property="Width" Value="240" />
                </ResponsiveRule>
            </FrameworkElement.ResponsiveRules>
        </FrameworkElement>)");
    ASSERT_EQ(roots.size(), 1u);

    const auto object = NeoWPF::UI::Markup::Xaml::BuildXamlTree(roots);
    const auto element =
        std::dynamic_pointer_cast<NeoWPF::UI::FrameworkElement>(object);
    ASSERT_NE(element, nullptr);
    ASSERT_EQ(element->GetResponsiveRules().size(), 1u);

    const auto& rule = element->GetResponsiveRules().front();
    ASSERT_NE(rule, nullptr);
    EXPECT_EQ(rule->GetScope(), NeoWPF::UI::ResponsiveScope::Window);
    ASSERT_EQ(rule->GetSetters().size(), 1u);
    EXPECT_EQ(rule->GetSetters().front()->GetProperty(), "Width");
}

TEST(ResponsiveLayoutTests, LayoutEnvironmentStoresSafeAreaKeyboardAndInputProfile)
{
    NeoWPF::UI::Layout::LayoutEnvironment environment;
    environment.contentWidth = 360.0;
    environment.contentHeight = 800.0;
    environment.safeArea = NeoWPF::UI::Thickness{20.0, 24.0, 20.0, 34.0};
    environment.keyboard = NeoWPF::UI::Thickness{0.0, 0.0, 0.0, 300.0};
    environment.pointer = NeoWPF::UI::Layout::PointerPrecision::Coarse;
    environment.hover = false;

    EXPECT_DOUBLE_EQ(environment.safeArea.Left, 20.0);
    EXPECT_DOUBLE_EQ(environment.safeArea.Top, 24.0);
    EXPECT_DOUBLE_EQ(environment.keyboard.Bottom, 300.0);
    EXPECT_EQ(environment.pointer, NeoWPF::UI::Layout::PointerPrecision::Coarse);
    EXPECT_FALSE(environment.hover);
}

TEST(ResponsiveLayoutTests, LayoutUtilitiesRepresentPositiveInfinityAsUnbounded)
{
    EXPECT_TRUE(NeoWPF::UI::Layout::IsBounded(360.0));
    EXPECT_FALSE(NeoWPF::UI::Layout::IsBounded(NeoWPF::UI::Layout::Unbounded));
    EXPECT_FALSE(NeoWPF::UI::Layout::IsBounded(std::numeric_limits<double>::quiet_NaN()));
}

TEST(ResponsiveLayoutTests, ResponsiveValueMasksLocalAndReturnsToLocal)
{
    class ResponsiveProbe final : public NeoWPF::UI::FrameworkElement {
    public:
        void SetResponsive(const NeoWPF::UI::Properties::DependencyProperty& property,
                           const std::any& value)
        {
            SetValueInternal(property, value, NeoWPF::UI::Properties::PropertyValueSource::Responsive);
        }

        void ClearResponsive(const NeoWPF::UI::Properties::DependencyProperty& property)
        {
            ClearResponsiveValue(property);
        }
    };

    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();
    static const auto& property =
        NeoWPF::UI::Properties::DependencyProperty::Register<ResponsiveProbe, int>(
            "Automated.ResponsivePrecedence",
            NeoWPF::UI::Properties::PropertyMetadata{10});

    ResponsiveProbe element;
    element.SetValue(property, 20);
    element.SetResponsive(property, 30);

    EXPECT_EQ(element.GetValue<int>(property), 30);
    EXPECT_EQ(element.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Responsive);

    element.ClearResponsive(property);
    EXPECT_EQ(element.GetValue<int>(property), 20);
    EXPECT_EQ(element.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Local);
}

TEST(ResponsiveLayoutTests, FrameworkElementMinMaxPropertiesHaveNonConstrainedDefaults)
{
    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();

    NeoWPF::UI::Panel parent;
    auto element = std::make_shared<NeoWPF::UI::FrameworkElement>();
    parent.AddChild(element);
    EXPECT_DOUBLE_EQ(element->GetMinWidth(), 0.0);
    EXPECT_DOUBLE_EQ(element->GetMaxWidth(), NeoWPF::UI::Layout::Unbounded);
    EXPECT_DOUBLE_EQ(element->GetMinHeight(), 0.0);
    EXPECT_DOUBLE_EQ(element->GetMaxHeight(), NeoWPF::UI::Layout::Unbounded);

    element->SetMinWidth(120.0);
    element->SetMaxWidth(320.0);
    element->SetMinHeight(48.0);
    element->SetMaxHeight(180.0);

    EXPECT_DOUBLE_EQ(element->GetMinWidth(), 120.0);
    EXPECT_DOUBLE_EQ(element->GetMaxWidth(), 320.0);
    EXPECT_DOUBLE_EQ(element->GetMinHeight(), 48.0);
    EXPECT_DOUBLE_EQ(element->GetMaxHeight(), 180.0);
}

TEST(ResponsiveLayoutTests, FrameworkElementResolveWidthAppliesMinMaxConstraints)
{
    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();

    NeoWPF::UI::Panel parent;
    auto element = std::make_shared<NeoWPF::UI::FrameworkElement>();
    parent.AddChild(element);
    element->SetWidthMode(NeoWPF::UI::LayoutSizeMode::Auto);
    element->SetWidth(500.0);
    element->SetMinWidth(200.0);
    element->SetMaxWidth(300.0);

    EXPECT_DOUBLE_EQ(element->ResolveWidth(600.0, 500.0), 300.0);

    element->SetWidth(100.0);
    EXPECT_DOUBLE_EQ(element->ResolveWidth(600.0, 100.0), 200.0);
}

TEST(ResponsiveLayoutTests, ResponsiveRuleUsesInclusiveMinimumAndExclusiveMaximum)
{
    NeoWPF::UI::ResponsiveRule rule;
    rule.SetMinWidth(360.0);
    rule.SetMaxWidth(800.0);
    rule.SetMinHeight(500.0);
    rule.SetMaxHeight(900.0);

    EXPECT_TRUE(rule.Matches(360.0, 500.0));
    EXPECT_TRUE(rule.Matches(799.999, 899.999));
    EXPECT_FALSE(rule.Matches(800.0, 600.0));
    EXPECT_FALSE(rule.Matches(600.0, 900.0));
}

TEST(ResponsiveLayoutTests, ResponsiveRuleRejectsUnboundedAndNonPositiveScopes)
{
    NeoWPF::UI::ResponsiveRule rule;
    rule.SetMinWidth(900.0);

    EXPECT_FALSE(rule.Matches(NeoWPF::UI::Layout::Unbounded, 800.0));
    EXPECT_FALSE(rule.Matches(800.0, 0.0));
    EXPECT_FALSE(rule.Matches(0.0, 800.0));
}

TEST(ResponsiveLayoutTests, NamedDescendantLookupFindsOnlyHostDescendants)
{
    NeoWPF::UI::Panel host;
    auto target = std::make_shared<NeoWPF::UI::FrameworkElement>();
    target->SetName("Content");
    host.AddChild(target);

    EXPECT_EQ(host.FindNameDescendant("Content"), target.get());
    EXPECT_EQ(host.FindNameDescendant("Missing"), nullptr);
}

TEST(ResponsiveLayoutTests, GridAttachedColumnAndSpanValuesClearAtZeroAndOne)
{
    NeoWPF::UI::Grid grid;
    auto child = std::make_shared<NeoWPF::UI::FrameworkElement>();
    grid.AddChild(child);

    grid.SetColumn(child.get(), 0);
    NeoWPF::UI::Grid::SetColumnSpan(child.get(), 1);

    EXPECT_EQ(grid.GetColumn(child.get()), 0);
    EXPECT_EQ(NeoWPF::UI::Grid::GetColumnSpan(child.get()), 1);
}

TEST(ResponsiveLayoutTests, StyleForwardsResponsiveRulesToFrameworkElement)
{
    NeoWPF::UI::Styling::Style style;
    style.SetTargetType(std::type_index(typeid(NeoWPF::UI::FrameworkElement)));

    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    style.AddResponsiveRule(rule);

    NeoWPF::UI::FrameworkElement element;
    style.ApplyTo(element);
    EXPECT_EQ(element.GetResponsiveRules().size(), 1u);
    EXPECT_EQ(element.GetResponsiveRules().front(), rule);
}

TEST(ResponsiveLayoutTests, RemovingRuleClearsPreviouslyAppliedResponsiveValues)
{
    NeoWPF::UI::Properties::RegisterFrameworkElementProperties();
    NeoWPF::UI::FrameworkElement element;
    element.SetWidth(100.0);

    auto rule = std::make_shared<NeoWPF::UI::ResponsiveRule>();
    rule->SetMinWidth(0.0);
    rule->SetMaxWidth(500.0);
    rule->SetMinHeight(0.0);
    rule->SetMaxHeight(500.0);
    rule->AddSetter(std::make_shared<NeoWPF::UI::Styling::Triggers::Setter>(
        "Width", 240.0));
    element.AddResponsiveRule(rule);

    element.Measure(400.0, 400.0);
    EXPECT_DOUBLE_EQ(element.GetWidth(), 240.0);

    element.RemoveResponsiveRule(rule);
    EXPECT_DOUBLE_EQ(element.GetWidth(), 100.0);
}

TEST(ResponsiveLayoutTests, GridDefinitionsExposeMinMaxProperties)
{
    NeoWPF::UI::Properties::RegisterColumnDefinitionProperties();
    NeoWPF::UI::Properties::RegisterRowDefinitionProperties();

    NeoWPF::UI::ColumnDefinition column;
    column.SetMinWidth(200.0);
    column.SetMaxWidth(320.0);
    EXPECT_DOUBLE_EQ(column.GetMinWidth(), 200.0);
    EXPECT_DOUBLE_EQ(column.GetMaxWidth(), 320.0);

    NeoWPF::UI::RowDefinition row;
    row.SetMinHeight(48.0);
    row.SetMaxHeight(180.0);
    EXPECT_DOUBLE_EQ(row.GetMinHeight(), 48.0);
    EXPECT_DOUBLE_EQ(row.GetMaxHeight(), 180.0);
}

}
