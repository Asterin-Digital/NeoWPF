#include <gtest/gtest.h>

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