#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Markup/Builder/Builder.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <any>
#include <string>

namespace {

struct TestObject : NeoWPF::UI::Properties::DependencyObject {
};

struct DerivedTestObject : TestObject {
};

int callbackCount = 0;
int callbackOldValue = -1;
int callbackNewValue = -1;

const auto& IntProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<TestObject, int>(
        "Automated.IntProperty",
        PropertyMetadata{42});
    return property;
}

const auto& CallbackProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<TestObject, int>(
        "Automated.CallbackProperty",
        PropertyMetadata{
            0,
            [](DependencyObject*, const DependencyPropertyChangedEventArgs& args) {
                ++callbackCount;
                callbackOldValue = std::any_cast<int>(args.OldValue);
                callbackNewValue = std::any_cast<int>(args.NewValue);
            }});
    return property;
}

TEST(DependencyPropertyTests, ReturnsMetadataAndDefaultValue)
{
    TestObject object;
    const auto& property = IntProperty();

    EXPECT_EQ(property.GetName(), "Automated.IntProperty");
    EXPECT_EQ(property.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(object.GetValue<int>(property), 42);
    EXPECT_EQ(object.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
}

TEST(DependencyPropertyTests, SetsReadsAndClearsLocalValue)
{
    TestObject object;
    const auto& property = IntProperty();

    object.SetValue(property, 99);
    EXPECT_EQ(object.GetValue<int>(property), 99);
    EXPECT_EQ(object.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Local);
    ASSERT_TRUE(object.ReadLocalValue(property).has_value());
    EXPECT_EQ(std::any_cast<int>(object.ReadLocalValue(property)), 99);

    object.ClearValue(property);

    EXPECT_EQ(object.GetValue<int>(property), 42);
    EXPECT_EQ(object.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Default);
    EXPECT_FALSE(object.ReadLocalValue(property).has_value());
}

TEST(DependencyPropertyTests, InvokesChangedCallbackWithOldAndNewValues)
{
    TestObject object;
    callbackCount = 0;
    callbackOldValue = -1;
    callbackNewValue = -1;
    const auto& property = CallbackProperty();

    object.SetValue(property, 7);

    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(callbackOldValue, 0);
    EXPECT_EQ(callbackNewValue, 7);
    EXPECT_EQ(object.GetValue<int>(property), 7);
}

TEST(DependencyPropertyTests, FindsPropertiesByExactAndDerivedOwner)
{
    TestObject object;
    DerivedTestObject derivedObject;

    EXPECT_EQ(object.FindDependencyProperty("Automated.IntProperty"), &IntProperty());
    EXPECT_EQ(derivedObject.FindDependencyProperty("Automated.IntProperty"), &IntProperty());
    EXPECT_EQ(object.FindDependencyProperty("Automated.DoesNotExist"), nullptr);
}

TEST(DependencyPropertyTests, ButtonExposesStaticPropertyHandles)
{
    const auto& textProperty = NeoWPF::UI::Button::TextProperty();

    EXPECT_EQ(textProperty.GetName(), "Text");
    EXPECT_EQ(textProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Button)));
    EXPECT_EQ(textProperty.GetPropertyType(), std::type_index(typeid(std::wstring)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(NeoWPF::UI::Button)), &textProperty);
}

TEST(DependencyPropertyTests, TextBlockExposesStaticPropertyHandles)
{
    const auto& textProperty = NeoWPF::UI::TextBlock::TextProperty();
    const auto& fontSizeProperty = NeoWPF::UI::TextBlock::FontSizeProperty();

    EXPECT_EQ(textProperty.GetName(), "Text");
    EXPECT_EQ(textProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::TextBlock)));
    EXPECT_EQ(textProperty.GetPropertyType(), std::type_index(typeid(std::wstring)));
    EXPECT_EQ(fontSizeProperty.GetName(), "FontSize");
    EXPECT_EQ(fontSizeProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(NeoWPF::UI::TextBlock)), &textProperty);
}

TEST(DependencyPropertyTests, TextBoxExposesStaticPropertyHandles)
{
    const auto& textProperty = NeoWPF::UI::TextBox::TextProperty();
    const auto& placeholderTextProperty = NeoWPF::UI::TextBox::PlaceholderTextProperty();
    const auto& fontSizeProperty = NeoWPF::UI::TextBox::FontSizeProperty();

    EXPECT_EQ(textProperty.GetName(), "Text");
    EXPECT_EQ(textProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::TextBox)));
    EXPECT_EQ(textProperty.GetPropertyType(), std::type_index(typeid(std::wstring)));
    EXPECT_EQ(placeholderTextProperty.GetName(), "PlaceholderText");
    EXPECT_EQ(fontSizeProperty.GetName(), "FontSize");
    EXPECT_EQ(fontSizeProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(NeoWPF::UI::TextBox)), &textProperty);
}

TEST(DependencyPropertyTests, UIElementExposesStaticPropertyHandles)
{
    const auto& visibilityProperty = NeoWPF::UI::UIElement::VisibilityProperty();
    const auto& isEnabledProperty = NeoWPF::UI::UIElement::IsEnabledProperty();
    const auto& opacityProperty = NeoWPF::UI::UIElement::OpacityProperty();

    EXPECT_EQ(visibilityProperty.GetName(), "Visibility");
    EXPECT_EQ(visibilityProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::UIElement)));
    EXPECT_EQ(visibilityProperty.GetPropertyType(), std::type_index(typeid(bool)));
    EXPECT_EQ(isEnabledProperty.GetName(), "IsEnabled");
    EXPECT_EQ(opacityProperty.GetName(), "Opacity");
    EXPECT_EQ(opacityProperty.GetPropertyType(), std::type_index(typeid(float)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Visibility", typeid(NeoWPF::UI::UIElement)), &visibilityProperty);
}

TEST(DependencyPropertyTests, FrameworkElementExposesStaticPropertyHandles)
{
    const auto& widthProperty = NeoWPF::UI::FrameworkElement::WidthProperty();
    const auto& heightProperty = NeoWPF::UI::FrameworkElement::HeightProperty();
    const auto& marginProperty = NeoWPF::UI::FrameworkElement::MarginProperty();
    const auto& dataContextProperty = NeoWPF::UI::FrameworkElement::DataContextProperty();

    EXPECT_EQ(widthProperty.GetName(), "Width");
    EXPECT_EQ(widthProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::FrameworkElement)));
    EXPECT_EQ(widthProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(heightProperty.GetName(), "Height");
    EXPECT_EQ(marginProperty.GetName(), "Margin");
    EXPECT_EQ(marginProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(dataContextProperty.GetName(), "DataContext");
    EXPECT_EQ(dataContextProperty.GetPropertyType(), std::type_index(typeid(std::any)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Width", typeid(NeoWPF::UI::FrameworkElement)), &widthProperty);
}

TEST(DependencyPropertyTests, ControlExposesStaticPropertyHandles)
{
    const auto& paddingProperty = NeoWPF::UI::Control::PaddingProperty();
    const auto& backgroundColorProperty = NeoWPF::UI::Control::BackgroundColorProperty();
    const auto& borderThicknessProperty = NeoWPF::UI::Control::BorderThicknessProperty();

    EXPECT_EQ(paddingProperty.GetName(), "Padding");
    EXPECT_EQ(paddingProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Control)));
    EXPECT_EQ(paddingProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(backgroundColorProperty.GetName(), "Background");
    EXPECT_EQ(backgroundColorProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Color)));
    EXPECT_EQ(borderThicknessProperty.GetName(), "BorderThickness");
    EXPECT_EQ(borderThicknessProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(NeoWPF::UI::Control)), &paddingProperty);
}

TEST(DependencyPropertyTests, CheckboxExposesStaticPropertyHandles)
{
    const auto& textProperty = NeoWPF::UI::Checkbox::TextProperty();
    const auto& textColorProperty = NeoWPF::UI::Checkbox::TextColorProperty();
    const auto& fontSizeProperty = NeoWPF::UI::Checkbox::FontSizeProperty();
    const auto& isCheckedProperty = NeoWPF::UI::Checkbox::IsCheckedProperty();

    EXPECT_EQ(textProperty.GetName(), "Text");
    EXPECT_EQ(textProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Checkbox)));
    EXPECT_EQ(textProperty.GetPropertyType(), std::type_index(typeid(std::wstring)));
    EXPECT_EQ(textColorProperty.GetName(), "Foreground");
    EXPECT_EQ(textColorProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Color)));
    EXPECT_EQ(fontSizeProperty.GetName(), "FontSize");
    EXPECT_EQ(fontSizeProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(isCheckedProperty.GetName(), "IsChecked");
    EXPECT_EQ(isCheckedProperty.GetPropertyType(), std::type_index(typeid(bool)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Text", typeid(NeoWPF::UI::Checkbox)), &textProperty);
}

TEST(DependencyPropertyTests, GridExposesAttachedPropertyHandles)
{
    const auto& rowProperty = NeoWPF::UI::Grid::RowProperty();
    const auto& columnProperty = NeoWPF::UI::Grid::ColumnProperty();
    const auto& rowSpanProperty = NeoWPF::UI::Grid::RowSpanProperty();
    const auto& columnSpanProperty = NeoWPF::UI::Grid::ColumnSpanProperty();

    EXPECT_EQ(rowProperty.GetName(), "Row");
    EXPECT_EQ(rowProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Grid)));
    EXPECT_EQ(rowProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_TRUE(rowProperty.IsAttached());
    EXPECT_EQ(rowProperty.GetAttachedTargetType(), std::type_index(typeid(NeoWPF::UI::Visual)));

    EXPECT_EQ(columnProperty.GetName(), "Column");
    EXPECT_EQ(rowSpanProperty.GetName(), "RowSpan");
    EXPECT_EQ(columnSpanProperty.GetName(), "ColumnSpan");
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindAttachedProperty("Row", typeid(NeoWPF::UI::Visual), typeid(NeoWPF::UI::Grid)), &rowProperty);
}

TEST(DependencyPropertyTests, BorderExposesStaticPropertyHandles)
{
    const auto& paddingProperty = NeoWPF::UI::Border::PaddingProperty();
    const auto& borderThicknessProperty = NeoWPF::UI::Border::BorderThicknessProperty();
    const auto& borderColorProperty = NeoWPF::UI::Border::BorderColorProperty();
    const auto& backgroundColorProperty = NeoWPF::UI::Border::BackgroundColorProperty();

    EXPECT_EQ(paddingProperty.GetName(), "Padding");
    EXPECT_EQ(paddingProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Border)));
    EXPECT_EQ(paddingProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(borderThicknessProperty.GetName(), "BorderThickness");
    EXPECT_EQ(borderThicknessProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(borderColorProperty.GetName(), "BorderBrush");
    EXPECT_EQ(borderColorProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Color)));
    EXPECT_EQ(backgroundColorProperty.GetName(), "Background");
    EXPECT_EQ(backgroundColorProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Color)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(NeoWPF::UI::Border)), &paddingProperty);
}

TEST(BuilderTests, FluentBuilderSetsPropertiesAndAttachesChildren)
{
    using namespace NeoWPF::UI::Builder;

    bool clicked = false;
    auto root = Build<NeoWPF::UI::StackPanel>()
        .Set(NeoWPF::UI::Panel::BackgroundColorProperty(), NeoWPF::UI::Color::FromArgb(0xFF112233u))
        .Children(
            Build<NeoWPF::UI::TextBlock>().Set(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Hello")).Name("Message"),
            Build<NeoWPF::UI::Button>()
                .Set(NeoWPF::UI::Button::TextProperty(), std::wstring(L"Click"))
                .Name("Submit")
                .On(&NeoWPF::UI::ButtonBase::Click, [&clicked]() { clicked = true; })
        );

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->GetValue<NeoWPF::UI::Color>(NeoWPF::UI::Panel::BackgroundColorProperty()), NeoWPF::UI::Color::FromArgb(0xFF112233u));
    EXPECT_EQ(root->GetVisualChildren().size(), 2u);

    const auto children = root->GetVisualChildren();
    ASSERT_GE(children.size(), 2u);

    auto* textBlock = dynamic_cast<NeoWPF::UI::TextBlock*>(children[0]);
    auto* button = dynamic_cast<NeoWPF::UI::Button*>(children[1]);
    ASSERT_NE(textBlock, nullptr);
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(textBlock->GetValue<std::wstring>(NeoWPF::UI::TextBlock::TextProperty()), std::wstring(L"Hello"));
    EXPECT_EQ(button->GetValue<std::wstring>(NeoWPF::UI::Button::TextProperty()), std::wstring(L"Click"));
    EXPECT_EQ(button->GetValue<std::wstring>(NeoWPF::UI::Button::TextProperty()), std::wstring(L"Click"));

    button->Click.Invoke();
    EXPECT_TRUE(clicked);
}

TEST(DependencyPropertyTests, PanelExposesStaticPropertyHandles)
{
    const auto& backgroundColorProperty = NeoWPF::UI::Panel::BackgroundColorProperty();

    EXPECT_EQ(backgroundColorProperty.GetName(), "Background");
    EXPECT_EQ(backgroundColorProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Panel)));
    EXPECT_EQ(backgroundColorProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Color)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Background", typeid(NeoWPF::UI::Panel)), &backgroundColorProperty);
}

TEST(DependencyPropertyTests, ScrollViewerExposesStaticPropertyHandles)
{
    const auto& horizontalOffsetProperty = NeoWPF::UI::ScrollViewer::HorizontalOffsetProperty();
    const auto& verticalOffsetProperty = NeoWPF::UI::ScrollViewer::VerticalOffsetProperty();
    const auto& horizontalVisibilityProperty = NeoWPF::UI::ScrollViewer::HorizontalScrollBarVisibilityProperty();
    const auto& verticalVisibilityProperty = NeoWPF::UI::ScrollViewer::VerticalScrollBarVisibilityProperty();

    EXPECT_EQ(horizontalOffsetProperty.GetName(), "HorizontalOffset");
    EXPECT_EQ(horizontalOffsetProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::ScrollViewer)));
    EXPECT_EQ(horizontalOffsetProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(verticalOffsetProperty.GetName(), "VerticalOffset");
    EXPECT_EQ(verticalOffsetProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(horizontalVisibilityProperty.GetName(), "HorizontalScrollBarVisibility");
    EXPECT_EQ(horizontalVisibilityProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::ScrollBarVisibility)));
    EXPECT_EQ(verticalVisibilityProperty.GetName(), "VerticalScrollBarVisibility");
    EXPECT_EQ(verticalVisibilityProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::ScrollBarVisibility)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalOffset", typeid(NeoWPF::UI::ScrollViewer)), &horizontalOffsetProperty);
}

TEST(DependencyPropertyTests, AnimationTakesPrecedenceAndCanBeCleared)
{
    TestObject object;
    const auto& property = IntProperty();

    object.SetValue(property, 10);
    object.SetValueAnimated(property, 20);
    EXPECT_EQ(object.GetValue<int>(property), 20);
    EXPECT_EQ(object.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Animation);

    object.ClearAnimatedValue(property);

    EXPECT_EQ(object.GetValue<int>(property), 10);
    EXPECT_EQ(object.GetValueSource(property),
              NeoWPF::UI::Properties::PropertyValueSource::Local);
}

}