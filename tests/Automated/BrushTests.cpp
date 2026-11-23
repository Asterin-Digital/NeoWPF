#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Colors.h>
#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace {

TEST(BrushTests, RegistersWpfShapedBaseAndSolidProperties)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    EXPECT_EQ(NeoWPF::UI::Brush::OpacityProperty().GetPropertyType(), typeid(double));
    EXPECT_EQ(NeoWPF::UI::Brush::TransformProperty().GetPropertyType(),
              typeid(std::shared_ptr<NeoWPF::UI::Core::Transform>));
    EXPECT_EQ(NeoWPF::UI::Brush::RelativeTransformProperty().GetPropertyType(),
              typeid(std::shared_ptr<NeoWPF::UI::Core::Transform>));
    EXPECT_EQ(NeoWPF::UI::SolidColorBrush::ArgbProperty().GetPropertyType(),
              typeid(std::uint32_t));
}

TEST(BrushTests, ProvidesDefaultsAndStoresSolidBrushValues)
{
    NeoWPF::UI::SolidColorBrush brush;

    EXPECT_DOUBLE_EQ(brush.GetOpacity(), 1.0);
    EXPECT_EQ(brush.GetArgb(), 0u);
    EXPECT_FALSE(brush.GetTransform());
    EXPECT_FALSE(brush.GetRelativeTransform());

    constexpr std::uint32_t argb = 0xFF123456u;
    brush.SetArgb(argb);
    brush.SetOpacity(0.5);

    EXPECT_EQ(brush.GetArgb(), argb);
    EXPECT_DOUBLE_EQ(brush.GetOpacity(), 0.5);
}

TEST(BrushTests, ColorPresetsAreSolidBrushes)
{
    static_assert(std::is_same_v<decltype(NeoWPF::UI::Colors::Red()),
                                 std::shared_ptr<NeoWPF::UI::Brush>>);

    const auto red = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(
        NeoWPF::UI::Colors::Red());
    ASSERT_NE(red, nullptr);
    EXPECT_EQ(red->GetArgb(), 0xFFFF0000u);

    const auto transparent = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(
        NeoWPF::UI::Colors::Transparent());
    ASSERT_NE(transparent, nullptr);
    EXPECT_EQ(transparent->GetArgb(), 0x00000000u);
}

TEST(BrushTests, NotifiesAndCanRemoveChangedHandlersDuringNotification)
{
    NeoWPF::UI::SolidColorBrush brush;
    std::size_t notificationCount = 0;
    std::size_t handlerId = 0;
    handlerId = brush.AddChangedHandler([&] {
        ++notificationCount;
        brush.RemoveChangedHandler(handlerId);
    });

    brush.SetArgb(0xFFFF0000u);
    brush.SetOpacity(0.25);

    EXPECT_EQ(notificationCount, 1u);
}

TEST(BrushTests, BrushSubscriptionTracksBrushAndTransformChanges)
{
    auto brush = std::make_shared<NeoWPF::UI::SolidColorBrush>();
    auto transform = std::make_shared<NeoWPF::UI::TranslateTransform>();
    brush->SetTransform(transform);

    std::size_t notificationCount = 0;
    {
        NeoWPF::UI::BrushSubscription subscription;
        subscription.Subscribe(brush, [&] {
            ++notificationCount;
        });

        brush->SetOpacity(0.5);
        transform->SetX(10.0f);
        EXPECT_EQ(notificationCount, 2u);
    }

    brush->SetOpacity(0.75);
    EXPECT_EQ(notificationCount, 2u);
}

TEST(BrushTests, GradientBrushesOwnIndependentStopCollections)
{
    auto first = std::make_shared<NeoWPF::UI::LinearGradientBrush>();
    auto second = std::make_shared<NeoWPF::UI::LinearGradientBrush>();

    ASSERT_NE(first->GetGradientStops(), nullptr);
    ASSERT_NE(second->GetGradientStops(), nullptr);
    EXPECT_NE(first->GetGradientStops(), second->GetGradientStops());

    std::size_t notificationCount = 0;
    NeoWPF::UI::BrushSubscription subscription;
    subscription.Subscribe(first, [&] { ++notificationCount; });

    auto stop = std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFFFF0000u), 0.25);
    first->GetGradientStops()->Add(stop);
    stop->SetOffset(0.75);
    EXPECT_EQ(notificationCount, 2u);
    EXPECT_TRUE(second->GetGradientStops()->empty());
}

TEST(BrushTests, LinearGradientResolvesSortedClampedStopsAndMapping)
{
    NeoWPF::UI::LinearGradientBrush brush;
    brush.SetStartPoint({0.0, 0.0});
    brush.SetEndPoint({1.0, 0.0});
    brush.SetOpacity(0.5);
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF0000FFu), 2.0));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFFFF0000u), -1.0));

    NeoWPF::Rendering::CommandList commands;
    const auto paint = brush.ResolvePaint(commands, 10.0, 20.0, 100.0, 50.0);

    ASSERT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::LinearGradient);
    ASSERT_EQ(commands.gradients.size(), 1u);
    ASSERT_EQ(commands.gradientStops.size(), 2u);
    EXPECT_EQ(commands.gradientStops[0].offset, 0.0f);
    EXPECT_EQ(commands.gradientStops[0].color, 0xFFFF0000u);
    EXPECT_EQ(commands.gradientStops[1].offset, 1.0f);
    EXPECT_EQ(commands.gradientStops[1].color, 0xFF0000FFu);
    EXPECT_NEAR(commands.gradients[0].mapping[0], 0.01, 1e-12);
    EXPECT_NEAR(commands.gradients[0].mapping[4], -0.1, 1e-12);
    EXPECT_FLOAT_EQ(commands.gradients[0].opacity, 0.5f);
}

TEST(BrushTests, LinearGallerySizeMapsToZeroAndOne)
{
    NeoWPF::UI::LinearGradientBrush brush;
    brush.SetStartPoint({0.0, 0.0});
    brush.SetEndPoint({1.0, 1.0});
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF3A7BD5u), 0.0));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF8A51DCu), 1.0));

    NeoWPF::Rendering::CommandList commands;
    const auto paint = brush.ResolvePaint(
        commands, 300.0, 636.0, 180.0, 72.0);

    ASSERT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::LinearGradient);
    ASSERT_EQ(commands.gradients.size(), 1u);
    const auto& mapping = commands.gradients[0].mapping;
    const double coordinateAtStart =
        mapping[0] * 300.0 + mapping[2] * 636.0 + mapping[4];
    const double coordinateAtEnd =
        mapping[0] * 480.0 + mapping[2] * 708.0 + mapping[4];
    EXPECT_NEAR(coordinateAtStart, 0.0, 1e-12);
    EXPECT_NEAR(coordinateAtEnd, 1.0, 1e-12);
}

TEST(BrushTests, GradientMappingComposesRelativeAndAbsoluteTransforms)
{
    NeoWPF::UI::LinearGradientBrush brush;
    brush.SetStartPoint({0.0, 0.0});
    brush.SetEndPoint({1.0, 0.0});
    brush.SetRelativeTransform(std::make_shared<NeoWPF::UI::TranslateTransform>(0.25f, 0.0f));
    brush.SetTransform(std::make_shared<NeoWPF::UI::TranslateTransform>(10.0f, 0.0f));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFFFF0000u), 0.0));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF0000FFu), 1.0));

    NeoWPF::Rendering::CommandList commands;
    brush.ResolvePaint(commands, 10.0, 20.0, 100.0, 50.0);
    ASSERT_EQ(commands.gradients.size(), 1u);
    EXPECT_NEAR(commands.gradients[0].mapping[0], 0.01, 1e-12);
    EXPECT_NEAR(commands.gradients[0].mapping[4], -0.45, 1e-12);

    brush.SetMappingMode(NeoWPF::UI::BrushMappingMode::Absolute);
    commands.Clear();
    brush.ResolvePaint(commands, 10.0, 20.0, 100.0, 50.0);
    ASSERT_EQ(commands.gradients.size(), 1u);
    EXPECT_NEAR(commands.gradients[0].mapping[0], 1.0, 1e-12);
    EXPECT_NEAR(commands.gradients[0].mapping[4], -45.0, 1e-12);
}

TEST(BrushTests, GradientSnapshotTracksStopEditsAndDegenerateLinearGeometry)
{
    NeoWPF::UI::LinearGradientBrush brush;
    auto first = std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF112233u), 0.0);
    auto last = std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF445566u), 1.0);
    brush.GetGradientStops()->Add(first);
    brush.GetGradientStops()->Add(last);

    NeoWPF::Rendering::CommandList commands;
    const auto paint = brush.ResolvePaint(commands, 0.0, 0.0, 100.0, 100.0);
    ASSERT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::LinearGradient);
    EXPECT_EQ(commands.gradients.size(), 1u);

    first->SetColor(NeoWPF::UI::Color::FromArgb(0xFFABCDEFu));
    commands.Clear();
    const auto degeneratePaint = brush.ResolvePaint(commands, 0.0, 0.0, 100.0, 100.0);
    ASSERT_EQ(degeneratePaint.kind, NeoWPF::Rendering::Paint::Kind::LinearGradient);
    EXPECT_EQ(commands.gradientStops[0].color, 0xFFABCDEFu);

    brush.SetEndPoint(brush.GetStartPoint());
    commands.Clear();
    const auto solidPaint = brush.ResolvePaint(commands, 0.0, 0.0, 100.0, 100.0);
    EXPECT_EQ(solidPaint.kind, NeoWPF::Rendering::Paint::Kind::Solid);
    EXPECT_EQ(solidPaint.color, 0xFF445566u);
    EXPECT_TRUE(commands.gradients.empty());
}

TEST(BrushTests, GradientFallbackColorSamplesMidpoint)
{
    NeoWPF::UI::LinearGradientBrush brush;
    brush.SetOpacity(0.5);
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFFFF0000u), 0.0));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF0000FFu), 1.0));

    EXPECT_EQ(brush.ResolveFallbackColor(), 0x7F800080u);
}

TEST(BrushTests, RadialGradientResolvesGeometryAndRejectsSingularTransforms)
{
    NeoWPF::UI::RadialGradientBrush brush;
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu), 0.0));
    brush.GetGradientStops()->Add(std::make_shared<NeoWPF::UI::GradientStop>(
        NeoWPF::UI::Color::FromArgb(0xFF000000u), 1.0));

    NeoWPF::Rendering::CommandList commands;
    const auto paint = brush.ResolvePaint(commands, 0.0, 0.0, 200.0, 100.0);
    ASSERT_EQ(paint.kind, NeoWPF::Rendering::Paint::Kind::RadialGradient);
    ASSERT_EQ(commands.gradients.size(), 1u);
    EXPECT_NEAR(commands.gradients[0].mapping[0], 0.01, 1e-12);
    EXPECT_NEAR(commands.gradients[0].mapping[3], 0.02, 1e-12);

    brush.SetTransform(std::make_shared<NeoWPF::UI::ScaleTransform>(0.0f, 1.0f));
    commands.Clear();
    const auto invisiblePaint = brush.ResolvePaint(commands, 0.0, 0.0, 200.0, 100.0);
    EXPECT_TRUE(invisiblePaint.IsEmpty());
    EXPECT_TRUE(commands.gradients.empty());
}

TEST(BrushTests, RegistersSolidColorBrushAsAnInstantiableXamlType)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    const auto* type = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find("SolidColorBrush");
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->CppTypeName, "NeoWPF::UI::SolidColorBrush");
    ASSERT_TRUE(type->Factory);
    EXPECT_NE(std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(type->Factory()), nullptr);
}

}