#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

TEST(BrushTests, RegistersWpfShapedBaseAndSolidProperties)
{
    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    EXPECT_EQ(NeoWPF::UI::Brush::OpacityProperty().GetPropertyType(), typeid(double));
    EXPECT_EQ(NeoWPF::UI::Brush::TransformProperty().GetPropertyType(),
              typeid(std::shared_ptr<NeoWPF::UI::Core::Transform>));
    EXPECT_EQ(NeoWPF::UI::Brush::RelativeTransformProperty().GetPropertyType(),
              typeid(std::shared_ptr<NeoWPF::UI::Core::Transform>));
    EXPECT_EQ(NeoWPF::UI::SolidColorBrush::ColorProperty().GetPropertyType(),
              typeid(NeoWPF::UI::Color));
}

TEST(BrushTests, ProvidesDefaultsAndStoresSolidBrushValues)
{
    NeoWPF::UI::SolidColorBrush brush;

    EXPECT_DOUBLE_EQ(brush.GetOpacity(), 1.0);
    EXPECT_EQ(brush.GetColor(), NeoWPF::UI::Colors::Transparent);
    EXPECT_FALSE(brush.GetTransform());
    EXPECT_FALSE(brush.GetRelativeTransform());

    const auto color = NeoWPF::UI::Color::FromArgb(0xFF123456u);
    brush.SetColor(color);
    brush.SetOpacity(0.5);

    EXPECT_EQ(brush.GetColor(), color);
    EXPECT_DOUBLE_EQ(brush.GetOpacity(), 0.5);
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

    brush.SetColor(NeoWPF::UI::Colors::Red);
    brush.SetOpacity(0.25);

    EXPECT_EQ(notificationCount, 1u);
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