#include <gtest/gtest.h>

#include "Generated/MainWindow.xaml.g.h"
#include "Generated/App.xaml.g.h"
#include "Generated/RootWindow.xaml.g.h"
#include "Generated/ResourceWindow.xaml.g.h"
#include "Generated/StyledWindow.xaml.g.h"

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <any>

TEST(GeneratedComponentCompileTests, ApplicationRootInitializesInPlace)
{
    NeoCM::Tests::App app;

    app.InitializeComponent();
    app.InitializeComponent();
}

TEST(GeneratedComponentCompileTests, WindowRootInitializesContentInPlace)
{
    NeoCM::Tests::RootWindow window;

    window.InitializeComponent();
    EXPECT_EQ(window.GetTitle(), std::wstring(L"Generated Window"));
    const auto windowRoot = std::dynamic_pointer_cast<NeoWPF::UI::Grid>(window.GetContent());
    ASSERT_NE(windowRoot, nullptr);
    ASSERT_EQ(windowRoot->GetLogicalChildren().size(), 1u);
    EXPECT_EQ(windowRoot->GetLogicalChildren().front(), window.RootGrid.get());
    ASSERT_NE(window.Label, nullptr);
    EXPECT_EQ(window.Label->GetText(), L"Generated content");

    const auto content = window.GetContent();
    window.InitializeComponent();
    EXPECT_EQ(window.GetContent(), content);
}

TEST(GeneratedComponentCompileTests, BuilderOutputCompilesAndInitializes)
{
    NeoCM::Tests::MainWindow component;

    component.InitializeComponent();
    ASSERT_NE(component.Root, nullptr);
    ASSERT_NE(component.ApplyButton, nullptr);
    ASSERT_EQ(component.Root->GetLogicalChildren().size(), 2u);

    const auto root = component.Root;
    const auto button = component.ApplyButton;
    component.InitializeComponent();
    EXPECT_EQ(component.Root, root);
    EXPECT_EQ(component.ApplyButton, button);
}

TEST(GeneratedComponentCompileTests, ResourceBuilderRegistersNearestResourceScope)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    NeoCM::Tests::ResourceWindow component;

    component.InitializeComponent();
    ASSERT_NE(component.Root, nullptr);
    ASSERT_NE(component.Nested, nullptr);
    ASSERT_NE(component.Consumer, nullptr);
    ASSERT_EQ(component.Root->GetVisualChildren().size(), 1u);
    ASSERT_EQ(component.Nested->GetVisualChildren().size(), 1u);

    std::any outerValue;
    ASSERT_TRUE(component.Root->TryFindResource("Accent", outerValue));
    const auto outer = std::any_cast<std::shared_ptr<NeoWPF::UI::TextBlock>>(outerValue);
    ASSERT_NE(outer, nullptr);

    std::any innerValue;
    ASSERT_TRUE(component.Nested->TryFindResource("Accent", innerValue));
    const auto inner = std::any_cast<std::shared_ptr<NeoWPF::UI::TextBlock>>(innerValue);
    ASSERT_NE(inner, nullptr);
    EXPECT_NE(outer.get(), inner.get());
    EXPECT_EQ(component.Consumer->GetChild().get(), inner.get());

    const auto root = component.Root;
    const auto nested = component.Nested;
    const auto consumer = component.Consumer;
    component.InitializeComponent();
    EXPECT_EQ(component.Root, root);
    EXPECT_EQ(component.Nested, nested);
    EXPECT_EQ(component.Consumer, consumer);
}

TEST(GeneratedComponentCompileTests, GeneratedImplicitStyleInitializesAndRemainsStable)
{
    NeoWPF::Core::Application::Initialize();
    NeoCM::Tests::StyledWindow component;

    component.InitializeComponent();
    ASSERT_NE(component.Root, nullptr);
    ASSERT_NE(component.StyledButton, nullptr);
    ASSERT_EQ(component.Root->GetVisualChildren().size(), 1u);
    EXPECT_EQ(component.StyledButton->GetParent(), component.Root.get());
    std::shared_ptr<NeoWPF::UI::Styling::Style> implicitStyle;
    ASSERT_TRUE(component.Root->TryFindImplicitStyle(
        std::type_index(typeid(NeoWPF::UI::Button)), implicitStyle));
    ASSERT_NE(implicitStyle, nullptr);
    ASSERT_EQ(implicitStyle->GetSetters().size(), 1u);

    component.Root->Initialize();
    EXPECT_EQ(component.StyledButton->GetText(), L"Compiled implicit");
    EXPECT_EQ(component.StyledButton->GetValueSource(NeoWPF::UI::Button::TextProperty()),
        NeoWPF::UI::Properties::PropertyValueSource::Style);

    const auto root = component.Root;
    const auto button = component.StyledButton;
    component.InitializeComponent();
    EXPECT_EQ(component.Root, root);
    EXPECT_EQ(component.StyledButton, button);
    EXPECT_EQ(component.StyledButton->GetText(), L"Compiled implicit");
    NeoWPF::Core::Application::Shutdown();
}