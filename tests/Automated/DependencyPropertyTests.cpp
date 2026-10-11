#include <gtest/gtest.h>

#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Markup/Builder/Builder.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

#include <any>
#include <string>

namespace {

struct TestObject : NeoWPF::UI::Properties::DependencyObject {
};

struct DerivedTestObject : TestObject {
};

const auto& BindingValueProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<TestObject, int>(
        "Automated.BindingValue",
        PropertyMetadata{0});
    return property;
}

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

const auto& StyleBindingProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<NeoWPF::UI::UIElement, int>(
        "Automated.StyleBindingProperty",
        PropertyMetadata{0});
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

const auto& CoercedProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<TestObject, int>(
        "Automated.CoercedProperty",
        PropertyMetadata{
            0,
            [](DependencyObject*, const DependencyPropertyChangedEventArgs& args) {
                ++callbackCount;
                callbackOldValue = std::any_cast<int>(args.OldValue);
                callbackNewValue = std::any_cast<int>(args.NewValue);
            },
            [](DependencyObject*, const std::any& value) -> std::any {
                return std::clamp(std::any_cast<int>(value), 0, 10);
            }});
    return property;
}

const auto& ThicknessEqualityProperty()
{
    using namespace NeoWPF::UI::Properties;
    static const auto& property = DependencyProperty::Register<TestObject, NeoWPF::UI::Thickness>(
        "Automated.ThicknessEqualityProperty",
        PropertyMetadata{
            NeoWPF::UI::Thickness(1),
            [](DependencyObject*, const DependencyPropertyChangedEventArgs&) {
                ++callbackCount;
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

TEST(DependencyPropertyTests, CoercesLocalAndAnimatedValuesBeforeCommit)
{
    TestObject object;
    callbackCount = 0;
    callbackOldValue = -1;
    callbackNewValue = -1;
    const auto& property = CoercedProperty();

    object.SetValue(property, 15);

    EXPECT_EQ(object.GetValue<int>(property), 10);
    EXPECT_EQ(callbackCount, 1);
    EXPECT_EQ(callbackOldValue, 0);
    EXPECT_EQ(callbackNewValue, 10);

    object.SetValueAnimated(property, std::any(7));

    EXPECT_EQ(object.GetValue<int>(property), 7);
    EXPECT_EQ(callbackCount, 2);
    EXPECT_EQ(callbackOldValue, 10);
    EXPECT_EQ(callbackNewValue, 7);
}

TEST(DependencyPropertyTests, ClearValueDoesNotNotifyWhenComparableValuesAreEqual)
{
    TestObject object;
    callbackCount = 0;
    const auto& property = ThicknessEqualityProperty();

    object.SetValue(property, NeoWPF::UI::Thickness(1));
    ASSERT_EQ(callbackCount, 1);

    callbackCount = 0;
    object.ClearValue(property);

    EXPECT_EQ(callbackCount, 0);
}

TEST(DependencyPropertyTests, ValuesEqualSharesTheDependencyPropertyComparisonRules)
{
    EXPECT_TRUE(NeoWPF::UI::Properties::ValuesEqual(std::any(true), std::any(true)));
    EXPECT_FALSE(NeoWPF::UI::Properties::ValuesEqual(std::any(true), std::any(false)));
    EXPECT_TRUE(NeoWPF::UI::Properties::ValuesEqual(std::any(42), std::any(42)));
    EXPECT_FALSE(NeoWPF::UI::Properties::ValuesEqual(std::any(42), std::any(7)));
    EXPECT_TRUE(NeoWPF::UI::Properties::ValuesEqual(std::any(std::wstring(L"item")), std::any(std::wstring(L"item"))));
    EXPECT_FALSE(NeoWPF::UI::Properties::ValuesEqual(std::any(std::string("item")), std::any(std::wstring(L"item"))));
    EXPECT_FALSE(NeoWPF::UI::Properties::ValuesEqual(std::any{}, std::any(42)));
    EXPECT_FALSE(NeoWPF::UI::Properties::ValuesEqual(std::any(42), std::any{}));
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

TEST(DependencyPropertyTests, ButtonTextSetterKeepsDependencyPropertyInSync)
{
    NeoWPF::UI::Button button;
    const auto& property = NeoWPF::UI::Button::TextProperty();

    button.SetText(L"Direct C++ text");

    EXPECT_EQ(button.GetText(), L"Direct C++ text");
    EXPECT_EQ(button.GetValue<std::wstring>(property), L"Direct C++ text");
    EXPECT_EQ(std::any_cast<std::wstring>(button.GetRawValue(property)), L"Direct C++ text");
}

TEST(DependencyPropertyTests, CheckboxPaddingPreservesThicknessInDependencyProperty)
{
    NeoWPF::UI::Checkbox checkbox;
    const auto& property = NeoWPF::UI::Checkbox::PaddingProperty();
    const NeoWPF::UI::Thickness padding(1, 2, 3, 4);

    checkbox.SetPadding(padding);

    EXPECT_EQ(checkbox.GetPadding(), padding);
    EXPECT_EQ(checkbox.GetValue<NeoWPF::UI::Thickness>(property), padding);
    EXPECT_EQ(std::any_cast<NeoWPF::UI::Thickness>(checkbox.GetRawValue(property)), padding);
}

TEST(DependencyPropertyTests, SupportsTwoWayBindingBetweenDependencyObjects)
{
    auto source = std::make_shared<TestObject>();
    TestObject target;
    const auto& property = BindingValueProperty();
    source->SetValue(property, 7);

    target.SetBinding(property,
        NeoWPF::UI::Data::Binding("Automated.BindingValue", NeoWPF::UI::Data::BindingMode::TwoWay)
            .WithSource(source));

    EXPECT_EQ(target.GetValue<int>(property), 7);

    source->SetValue(property, 11);
    EXPECT_EQ(target.GetValue<int>(property), 11);

    target.SetValue(property, 13);
    EXPECT_EQ(source->GetValue<int>(property), 13);

    target.ClearBinding(property);
    source->SetValue(property, 17);
    EXPECT_EQ(target.GetValue<int>(property), 0);
}

TEST(DependencyPropertyTests, DoesNotWriteBackOneWayDependencyObjectBinding)
{
    auto source = std::make_shared<TestObject>();
    TestObject target;
    const auto& property = BindingValueProperty();
    source->SetValue(property, 7);

    target.SetBinding(property,
        NeoWPF::UI::Data::Binding("Automated.BindingValue", NeoWPF::UI::Data::BindingMode::OneWay)
            .WithSource(source));

    target.SetValue(property, 13);
    EXPECT_EQ(source->GetValue<int>(property), 7);
}

TEST(DependencyPropertyTests, StyleApplicationPreservesBindingExpression)
{
    auto source = std::make_shared<NeoWPF::UI::UIElement>();
    NeoWPF::UI::UIElement target;
    const auto& property = StyleBindingProperty();
    source->SetValue(property, 7);

    target.SetBinding(property,
        NeoWPF::UI::Data::Binding("Automated.StyleBindingProperty")
            .WithSource(source));

    ASSERT_NE(NeoWPF::UI::Data::BindingOperations::GetBindingExpression(target, property), nullptr);
    EXPECT_EQ(target.GetValue<int>(property), 7);

    ASSERT_TRUE(target.SetStylePropertyValue("Automated.StyleBindingProperty", 13));

    EXPECT_NE(NeoWPF::UI::Data::BindingOperations::GetBindingExpression(target, property), nullptr);
    EXPECT_EQ(target.GetValue<int>(property), 7);
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
    EXPECT_EQ(widthProperty.GetPropertyType(), std::type_index(typeid(double)));
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
    const auto& backgroundProperty = NeoWPF::UI::Control::BackgroundProperty();
    const auto& borderBrushProperty = NeoWPF::UI::Control::BorderBrushProperty();
    const auto& borderThicknessProperty = NeoWPF::UI::Control::BorderThicknessProperty();

    EXPECT_EQ(paddingProperty.GetName(), "Padding");
    EXPECT_EQ(paddingProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Control)));
    EXPECT_EQ(paddingProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(backgroundProperty.GetName(), "Background");
    EXPECT_EQ(backgroundProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(borderBrushProperty.GetName(), "BorderBrush");
    EXPECT_EQ(borderBrushProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(borderThicknessProperty.GetName(), "BorderThickness");
    EXPECT_EQ(borderThicknessProperty.GetPropertyType(), std::type_index(typeid(double)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(NeoWPF::UI::Control)), &paddingProperty);
}

TEST(DependencyPropertyTests, CheckboxExposesStaticPropertyHandles)
{
    const auto& textProperty = NeoWPF::UI::Checkbox::TextProperty();
    const auto& foregroundProperty = NeoWPF::UI::Checkbox::ForegroundProperty();
    const auto& fontSizeProperty = NeoWPF::UI::Checkbox::FontSizeProperty();
    const auto& isCheckedProperty = NeoWPF::UI::Checkbox::IsCheckedProperty();

    EXPECT_EQ(textProperty.GetName(), "Text");
    EXPECT_EQ(textProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Checkbox)));
    EXPECT_EQ(textProperty.GetPropertyType(), std::type_index(typeid(std::wstring)));
    EXPECT_EQ(foregroundProperty.GetName(), "Foreground");
    EXPECT_EQ(foregroundProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
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
    const auto& borderBrushProperty = NeoWPF::UI::Control::BorderBrushProperty();
    const auto& backgroundProperty = NeoWPF::UI::Control::BackgroundProperty();

    EXPECT_EQ(paddingProperty.GetName(), "Padding");
    EXPECT_EQ(paddingProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Border)));
    EXPECT_EQ(paddingProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(borderThicknessProperty.GetName(), "BorderThickness");
    EXPECT_EQ(borderThicknessProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::Thickness)));
    EXPECT_EQ(borderBrushProperty.GetName(), "BorderBrush");
    EXPECT_EQ(borderBrushProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(backgroundProperty.GetName(), "Background");
    EXPECT_EQ(backgroundProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", typeid(NeoWPF::UI::Border)), &paddingProperty);
}

TEST(BuilderTests, FluentBuilderSetsPropertiesAndAttachesChildren)
{
    using namespace NeoWPF::UI::Builder;

    bool clicked = false;
    const std::shared_ptr<NeoWPF::UI::Brush> backgroundBrush =
        std::make_shared<NeoWPF::UI::SolidColorBrush>(0xFF112233u);
    auto root = Build<NeoWPF::UI::StackPanel>()
        .Set(NeoWPF::UI::Control::BackgroundProperty(), backgroundBrush)
        .Children(
            Build<NeoWPF::UI::TextBlock>().Set(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Hello")).Name("Message"),
            Build<NeoWPF::UI::Button>()
                .Set(NeoWPF::UI::Button::TextProperty(), std::wstring(L"Click"))
                .Name("Submit")
                .On(&NeoWPF::UI::ButtonBase::Click, [&clicked]() { clicked = true; })
        );

    ASSERT_NE(root, nullptr);
    const auto rootSolidBackground = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(root->GetBackgroundBrush());
    ASSERT_NE(rootSolidBackground, nullptr);
    EXPECT_EQ(rootSolidBackground->GetArgb(), 0xFF112233u);
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
    NeoWPF::UI::Panel panel;
    const auto& backgroundProperty = NeoWPF::UI::Control::BackgroundProperty();

    EXPECT_EQ(backgroundProperty.GetName(), "Background");
    EXPECT_EQ(backgroundProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Control)));
    EXPECT_EQ(backgroundProperty.GetPropertyType(), std::type_index(typeid(std::shared_ptr<NeoWPF::UI::Brush>)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Background", panel), &backgroundProperty);
}

TEST(DependencyPropertyTests, ScrollViewerExposesStaticPropertyHandles)
{
    const auto& horizontalOffsetProperty = NeoWPF::UI::ScrollViewer::HorizontalOffsetProperty();
    const auto& verticalOffsetProperty = NeoWPF::UI::ScrollViewer::VerticalOffsetProperty();
    const auto& horizontalVisibilityProperty = NeoWPF::UI::ScrollViewer::HorizontalScrollBarVisibilityProperty();
    const auto& verticalVisibilityProperty = NeoWPF::UI::ScrollViewer::VerticalScrollBarVisibilityProperty();

    EXPECT_EQ(horizontalOffsetProperty.GetName(), "HorizontalOffset");
    EXPECT_EQ(horizontalOffsetProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::ScrollViewer)));
    EXPECT_EQ(horizontalOffsetProperty.GetPropertyType(), std::type_index(typeid(double)));
    EXPECT_EQ(verticalOffsetProperty.GetName(), "VerticalOffset");
    EXPECT_EQ(verticalOffsetProperty.GetPropertyType(), std::type_index(typeid(double)));
    EXPECT_EQ(horizontalVisibilityProperty.GetName(), "HorizontalScrollBarVisibility");
    EXPECT_EQ(horizontalVisibilityProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::ScrollBarVisibility)));
    EXPECT_EQ(verticalVisibilityProperty.GetName(), "VerticalScrollBarVisibility");
    EXPECT_EQ(verticalVisibilityProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::ScrollBarVisibility)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalOffset", typeid(NeoWPF::UI::ScrollViewer)), &horizontalOffsetProperty);
}

TEST(DependencyPropertyTests, ScrollViewerConstructorDoesNotAttachItsPresenterTwice)
{
    NeoWPF::UI::ScrollViewer scrollViewer;

    EXPECT_EQ(scrollViewer.GetHorizontalScrollBarVisibility(), NeoWPF::UI::ScrollBarVisibility::Auto);
    EXPECT_EQ(scrollViewer.GetVerticalScrollBarVisibility(), NeoWPF::UI::ScrollBarVisibility::Auto);
}

TEST(DependencyPropertyTests, SelectorExposesStaticPropertyHandles)
{
    const auto& selectedIndexProperty = NeoWPF::UI::Selector::SelectedIndexProperty();
    const auto& selectedItemProperty = NeoWPF::UI::Selector::SelectedItemProperty();
    const auto& selectionModeProperty = NeoWPF::UI::Selector::SelectionModeProperty();

    EXPECT_EQ(selectedIndexProperty.GetName(), "SelectedIndex");
    EXPECT_EQ(selectedIndexProperty.GetOwnerType(), std::type_index(typeid(NeoWPF::UI::Selector)));
    EXPECT_EQ(selectedIndexProperty.GetPropertyType(), std::type_index(typeid(int)));
    EXPECT_EQ(selectedItemProperty.GetName(), "SelectedItem");
    EXPECT_EQ(selectedItemProperty.GetPropertyType(), std::type_index(typeid(std::any)));
    EXPECT_EQ(selectionModeProperty.GetName(), "SelectionMode");
    EXPECT_EQ(selectionModeProperty.GetPropertyType(), std::type_index(typeid(NeoWPF::UI::SelectionMode)));
    EXPECT_EQ(NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("SelectionMode", typeid(NeoWPF::UI::Selector)), &selectionModeProperty);
}

TEST(DependencyPropertyTests, SelectorSelectionModeRejectsInvalidStrings)
{
    NeoWPF::UI::Selector selector;
    const auto& selectionModeProperty = NeoWPF::UI::Selector::SelectionModeProperty();

    std::any parsed;
    EXPECT_TRUE(selectionModeProperty.TryParseValue("multiple", parsed));
    EXPECT_EQ(std::any_cast<NeoWPF::UI::SelectionMode>(parsed), NeoWPF::UI::SelectionMode::Multiple);

    selector.SetValue(selectionModeProperty, NeoWPF::UI::SelectionMode::Extended);
    EXPECT_EQ(selector.GetSelectionMode(), NeoWPF::UI::SelectionMode::Extended);

    EXPECT_FALSE(selectionModeProperty.TryParseValue("not-a-mode", parsed));
    EXPECT_FALSE(selectionModeProperty.TryParseValue("", parsed));
    EXPECT_FALSE(selectionModeProperty.TryParseValue("single,extended", parsed));
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

TEST(TypeConverterTests, ConvertsWideStringsToEscapedCode)
{
    std::string expression;
    const auto converter = NeoWPF::UI::Properties::GetWideStringConverter();

    ASSERT_TRUE(converter->TryConvertToCode("Apply\"Now", expression));
    EXPECT_EQ(expression, "std::wstring(L\"Apply\\\"Now\")");
}

TEST(TypeConverterTests, EscapesControlCharactersAndBackslashes)
{
    std::string expression;
    const auto converter = NeoWPF::UI::Properties::GetWideStringConverter();

    ASSERT_TRUE(converter->TryConvertToCode("A\\B\nC\tD\rE", expression));
    EXPECT_EQ(expression, "std::wstring(L\"A\\\\B\\nC\\tD\\rE\")");
}

TEST(TypeConverterTests, ConvertsScalarValuesToCode)
{
    std::string expression;
    EXPECT_TRUE(NeoWPF::UI::Properties::GetIntConverter()->TryConvertToCode("42", expression));
    EXPECT_EQ(expression, "42");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetBoolConverter()->TryConvertToCode("TRUE", expression));
    EXPECT_EQ(expression, "true");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetFloatConverter()->TryConvertToCode("1.5", expression));
    EXPECT_EQ(expression, "1.500000f");
}

TEST(TypeConverterTests, AcceptsTrimmedScalarValuesAndRejectsMalformedValues)
{
    std::string expression;
    EXPECT_TRUE(NeoWPF::UI::Properties::GetIntConverter()->TryConvertToCode("  -2147483648  ", expression));
    EXPECT_EQ(expression, "-2147483648");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetFloatConverter()->TryConvertToCode("  -0.25  ", expression));
    EXPECT_EQ(expression, "-0.250000f");

    EXPECT_FALSE(NeoWPF::UI::Properties::GetIntConverter()->TryConvertToCode("12px", expression));
    EXPECT_TRUE(expression.empty());
    EXPECT_FALSE(NeoWPF::UI::Properties::GetFloatConverter()->TryConvertToCode("1.0f", expression));
    EXPECT_TRUE(expression.empty());
}

TEST(TypeConverterTests, AcceptsBooleanCasingAndRejectsUnknownValues)
{
    std::string expression;
    const auto converter = NeoWPF::UI::Properties::GetBoolConverter();

    for (const auto& input : {"true", "TRUE", " True "}) {
        ASSERT_TRUE(converter->TryConvertToCode(input, expression)) << input;
        EXPECT_EQ(expression, "true");
    }
    for (const auto& input : {"false", "FALSE", " False "}) {
        ASSERT_TRUE(converter->TryConvertToCode(input, expression)) << input;
        EXPECT_EQ(expression, "false");
    }

    EXPECT_FALSE(converter->TryConvertToCode("yes", expression));
    EXPECT_TRUE(expression.empty());
}

TEST(TypeConverterTests, ConvertsAlignmentNamesToEnumCode)
{
    std::string expression;
    EXPECT_TRUE(NeoWPF::UI::Properties::GetHorizontalAlignmentConverter()->TryConvertToCode("center", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::HorizontalAlignment::Center");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetVerticalAlignmentConverter()->TryConvertToCode("BOTTOM", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::VerticalAlignment::Bottom");
}

TEST(TypeConverterTests, ConvertsSelectionModeNamesToEnumCode)
{
    std::string expression;
    EXPECT_TRUE(NeoWPF::UI::Properties::GetSelectionModeConverter()->TryConvertToCode("single", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::SelectionMode::Single");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetSelectionModeConverter()->TryConvertToCode("MULTIPLE", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::SelectionMode::Multiple");
    EXPECT_TRUE(NeoWPF::UI::Properties::GetSelectionModeConverter()->TryConvertToCode(" Extended ", expression));
    EXPECT_EQ(expression, "NeoWPF::UI::SelectionMode::Extended");

    EXPECT_FALSE(NeoWPF::UI::Properties::GetSelectionModeConverter()->TryConvertToCode("radial", expression));
    EXPECT_TRUE(expression.empty());
    EXPECT_FALSE(NeoWPF::UI::Properties::GetSelectionModeConverter()->TryConvertToCode("single,multiple", expression));
    EXPECT_TRUE(expression.empty());
}

TEST(TypeConverterTests, ConvertsAllAlignmentMembersCaseInsensitively)
{
    const auto horizontal = NeoWPF::UI::Properties::GetHorizontalAlignmentConverter();
    const auto vertical = NeoWPF::UI::Properties::GetVerticalAlignmentConverter();
    std::string expression;

    for (const auto& input : {"LEFT", "center", "RiGhT", "stretch"}) {
        EXPECT_TRUE(horizontal->TryConvertToCode(input, expression)) << input;
        EXPECT_EQ(expression.rfind("NeoWPF::UI::HorizontalAlignment::", 0), 0u);
    }
    for (const auto& input : {"TOP", "center", "BoTtOm", "stretch"}) {
        EXPECT_TRUE(vertical->TryConvertToCode(input, expression)) << input;
        EXPECT_EQ(expression.rfind("NeoWPF::UI::VerticalAlignment::", 0), 0u);
    }

    EXPECT_FALSE(horizontal->TryConvertToCode("middle", expression));
    EXPECT_TRUE(expression.empty());
    EXPECT_FALSE(vertical->TryConvertToCode("middle", expression));
    EXPECT_TRUE(expression.empty());
}

TEST(TypeConverterTests, ClearsExpressionWhenConversionFails)
{
    std::string expression = "stale code";

    EXPECT_FALSE(NeoWPF::UI::Properties::GetIntConverter()->TryConvertToCode("not-an-int", expression));
    EXPECT_TRUE(expression.empty());

    expression = "stale code";
    EXPECT_FALSE(NeoWPF::UI::Properties::GetHorizontalAlignmentConverter()->TryConvertToCode("diagonal", expression));
    EXPECT_TRUE(expression.empty());
}