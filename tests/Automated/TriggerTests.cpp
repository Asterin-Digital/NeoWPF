#include <gtest/gtest.h>

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Styling/Triggers/Trigger.h>
#include <NeoWPF/UI/Styling/Triggers/DataTrigger.h>
#include <NeoWPF/UI/Styling/Triggers/EventTrigger.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Styling/Triggers/TriggerCollection.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/BeginStoryboardAction.h>
#include <NeoWPF/UI/Styling/Triggers/Actions/StopStoryboardAction.h>

#include <any>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

namespace {

using namespace NeoWPF::UI;
using namespace NeoWPF::UI::Properties;
using namespace NeoWPF::UI::Styling::Triggers;

const auto& IntTriggerProperty()
{
    static const auto& property = DependencyProperty::Register<UIElement, int>(
        "Automated.IntTriggerProperty",
        PropertyMetadata{0});
    return property;
}

const auto& BoolTriggerProperty()
{
    static const auto& property = DependencyProperty::Register<UIElement, bool>(
        "Automated.BoolTriggerProperty",
        PropertyMetadata{false});
    return property;
}

const auto& StringTriggerProperty()
{
    static const auto& property = DependencyProperty::Register<UIElement, std::string>(
        "Automated.StringTriggerProperty",
        PropertyMetadata{std::string("default")});
    return property;
}

const auto& FloatTriggerProperty()
{
    static const auto& property = DependencyProperty::Register<UIElement, float>(
        "Automated.FloatTriggerProperty",
        PropertyMetadata{0.0f});
    return property;
}

}

// ============================================================================
// Trigger Tests
// ============================================================================

TEST(TriggerTests, ConstructorSetsPropertyAndValue)
{
    Trigger trigger("TestProperty", 42);

    EXPECT_EQ(trigger.GetProperty(), "TestProperty");
    EXPECT_EQ(std::any_cast<int>(trigger.GetValue()), 42);
}

TEST(TriggerTests, SetAndGetProperty)
{
    Trigger trigger;
    trigger.SetProperty("NewProperty");

    EXPECT_EQ(trigger.GetProperty(), "NewProperty");
}

TEST(TriggerTests, SetAndGetValue)
{
    Trigger trigger;
    trigger.SetValue(99);

    EXPECT_EQ(std::any_cast<int>(trigger.GetValue()), 99);
}

TEST(TriggerTests, EvaluateTriggerWhenPropertyMatches)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    Trigger trigger("Automated.IntTriggerProperty", 42);

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(TriggerTests, EvaluateTriggerWhenPropertyDoesNotMatch)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    Trigger trigger("Automated.IntTriggerProperty", 99);

    EXPECT_FALSE(trigger.Evaluate(&element));
}

TEST(TriggerTests, EvaluateTriggerWithBoolValue)
{
    UIElement element;
    element.SetValue(BoolTriggerProperty(), true);

    Trigger trigger("Automated.BoolTriggerProperty", true);

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(TriggerTests, EvaluateTriggerWithStringValue)
{
    UIElement element;
    element.SetValue(StringTriggerProperty(), std::string("active"));

    Trigger trigger("Automated.StringTriggerProperty", std::string("active"));

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(TriggerTests, EvaluateTriggerWithFloatValue)
{
    UIElement element;
    element.SetValue(FloatTriggerProperty(), 1.5f);

    Trigger trigger("Automated.FloatTriggerProperty", 1.5f);

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(TriggerTests, EvaluateReturnsFalseForNullElement)
{
    Trigger trigger("Property", 42);

    EXPECT_FALSE(trigger.Evaluate(nullptr));
}

TEST(TriggerTests, EvaluateReturnsFalseForNonexistentProperty)
{
    UIElement element;
    Trigger trigger("NonexistentProperty", 42);

    EXPECT_FALSE(trigger.Evaluate(&element));
}

// ============================================================================
// DataTrigger Tests
// ============================================================================

TEST(DataTriggerTests, ConstructorSetsBindingPathAndValue)
{
    DataTrigger trigger("data.property", 42);

    EXPECT_EQ(trigger.GetBindingPath(), "data.property");
    EXPECT_EQ(std::any_cast<int>(trigger.GetValue()), 42);
}

TEST(DataTriggerTests, SetAndGetBindingPath)
{
    DataTrigger trigger;
    trigger.SetBindingPath("NewPath");

    EXPECT_EQ(trigger.GetBindingPath(), "NewPath");
}

TEST(DataTriggerTests, EvaluateDataTriggerWhenPropertyMatches)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    DataTrigger trigger("Automated.IntTriggerProperty", 42);

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(DataTriggerTests, EvaluateDataTriggerWhenPropertyDoesNotMatch)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    DataTrigger trigger("Automated.IntTriggerProperty", 99);

    EXPECT_FALSE(trigger.Evaluate(&element));
}

TEST(DataTriggerTests, EvaluateReturnsFalseForNullElement)
{
    DataTrigger trigger("property", 42);

    EXPECT_FALSE(trigger.Evaluate(nullptr));
}

TEST(DataTriggerTests, EvaluateReturnsFalseForNonexistentProperty)
{
    UIElement element;
    DataTrigger trigger("NonexistentProperty", 42);

    EXPECT_FALSE(trigger.Evaluate(&element));
}

// ============================================================================
// Setter Tests
// ============================================================================

TEST(SetterTests, ConstructorSetsPropertyAndValue)
{
    Setter setter("PropertyName", 42);

    EXPECT_EQ(setter.GetProperty(), "PropertyName");
    EXPECT_EQ(std::any_cast<int>(setter.GetValue()), 42);
}

TEST(SetterTests, SetAndGetProperty)
{
    Setter setter;
    setter.SetProperty("NewProperty");

    EXPECT_EQ(setter.GetProperty(), "NewProperty");
}

TEST(SetterTests, SetAndGetValue)
{
    Setter setter;
    setter.SetValue(99);

    EXPECT_EQ(std::any_cast<int>(setter.GetValue()), 99);
}

TEST(SetterTests, ApplySetterUpdatesElementProperty)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 10);

    Setter setter("Automated.IntTriggerProperty", 42);
    setter.Apply(&element);

    EXPECT_EQ(element.GetValue<int>(IntTriggerProperty()), 42);
}

TEST(SetterTests, ApplySetterWithBoolValue)
{
    UIElement element;
    element.SetValue(BoolTriggerProperty(), false);

    Setter setter("Automated.BoolTriggerProperty", true);
    setter.Apply(&element);

    EXPECT_EQ(element.GetValue<bool>(BoolTriggerProperty()), true);
}

TEST(SetterTests, ApplySetterWithStringValue)
{
    UIElement element;
    element.SetValue(StringTriggerProperty(), std::string("old"));

    Setter setter("Automated.StringTriggerProperty", std::string("new"));
    setter.Apply(&element);

    EXPECT_EQ(element.GetValue<std::string>(StringTriggerProperty()), "new");
}

TEST(SetterTests, ApplySetterIgnoresNullElement)
{
    Setter setter("Property", 42);

    // Should not crash
    setter.Apply(nullptr);
}

TEST(SetterTests, ApplySetterIgnoresNonexistentProperty)
{
    UIElement element;
    Setter setter("NonexistentProperty", 42);

    // Should not crash or modify element
    setter.Apply(&element);
}

// ============================================================================
// TriggerBase Tests
// ============================================================================

TEST(TriggerBaseTests, AddSettersAndRetrieve)
{
    Trigger trigger;
    Setter setter1("Property1", 10);
    Setter setter2("Property2", 20);

    trigger.AddSetter(setter1);
    trigger.AddSetter(std::move(setter2));

    const auto& setters = trigger.GetSetters();
    EXPECT_EQ(setters.size(), 2);
    EXPECT_EQ(setters[0].GetProperty(), "Property1");
    EXPECT_EQ(setters[1].GetProperty(), "Property2");
}

TEST(TriggerBaseTests, ApplyAllSettersToElement)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 0);
    element.SetValue(BoolTriggerProperty(), false);

    Trigger trigger;
    trigger.AddSetter(Setter("Automated.IntTriggerProperty", 42));
    trigger.AddSetter(Setter("Automated.BoolTriggerProperty", true));

    trigger.Apply(&element);

    EXPECT_EQ(element.GetValue<int>(IntTriggerProperty()), 42);
    EXPECT_EQ(element.GetValue<bool>(BoolTriggerProperty()), true);
}

TEST(TriggerBaseTests, AttachAndDetachTarget)
{
    UIElement element;
    Trigger trigger;

    trigger.Attach(&element);
    EXPECT_EQ(trigger.GetTarget(), &element);

    trigger.Detach();
    EXPECT_EQ(trigger.GetTarget(), nullptr);
}

// ============================================================================
// EventTrigger Tests
// ============================================================================

TEST(EventTriggerTests, ConstructorSetsRoutedEventName)
{
    EventTrigger trigger("Click");

    EXPECT_EQ(trigger.GetRoutedEventName(), "Click");
}

TEST(EventTriggerTests, SetAndGetRoutedEventName)
{
    EventTrigger trigger;
    trigger.SetRoutedEventName("MouseEnter");

    EXPECT_EQ(trigger.GetRoutedEventName(), "MouseEnter");
}

TEST(EventTriggerTests, AddActionsAndRetrieve)
{
    EventTrigger trigger;
    auto action1 = std::make_shared<BeginStoryboardAction>(nullptr);
    auto action2 = std::make_shared<StopStoryboardAction>(0);

    trigger.AddAction(action1);
    trigger.AddAction(action2);

    const auto& actions = trigger.GetActions();
    EXPECT_EQ(actions.size(), 2);
    EXPECT_EQ(actions[0], action1);
    EXPECT_EQ(actions[1], action2);
}

TEST(EventTriggerTests, AddActionIgnoresNull)
{
    EventTrigger trigger;

    trigger.AddAction(nullptr);

    EXPECT_EQ(trigger.GetActions().size(), 0);
}

TEST(EventTriggerTests, EvaluateReturnsFalseForNullElement)
{
    EventTrigger trigger("Click");

    EXPECT_FALSE(trigger.Evaluate(nullptr));
}

TEST(EventTriggerTests, EvaluateReturnsFalseForEmptyEventName)
{
    UIElement element;
    EventTrigger trigger("");

    EXPECT_FALSE(trigger.Evaluate(&element));
}

TEST(EventTriggerTests, EvaluateReturnsTrueForValidElementAndEvent)
{
    UIElement element;
    EventTrigger trigger("Click");

    EXPECT_TRUE(trigger.Evaluate(&element));
}

TEST(EventTriggerTests, InvokeCallsAllActions)
{
    UIElement element;
    EventTrigger trigger;

    // Create a counter action for testing
    class CountingAction : public TriggerAction {
    public:
        mutable int invokeCount = 0;
        void Invoke(UIElement*) const override {
            ++invokeCount;
        }
    };

    auto action1 = std::make_shared<CountingAction>();
    auto action2 = std::make_shared<CountingAction>();

    trigger.AddAction(action1);
    trigger.AddAction(action2);

    trigger.Invoke(&element);

    EXPECT_EQ(action1->invokeCount, 1);
    EXPECT_EQ(action2->invokeCount, 1);
}

TEST(EventTriggerTests, AttachedTriggerInvokesOnMatchingRoutedEvent)
{
    UIElement element;
    class CountingAction : public TriggerAction {
    public:
        mutable int invokeCount = 0;
        void Invoke(UIElement*) const override {
            ++invokeCount;
        }
    };

    auto action = std::make_shared<CountingAction>();
    auto trigger = std::make_shared<EventTrigger>("MouseEnter");
    trigger->AddAction(action);

    element.AddEventTrigger(trigger);

    NeoWPF::Input::Core::RoutedMouseEventArgs args(NeoWPF::Input::Core::MouseEnterEvent(), 0, 0, 0);
    element.RaiseEvent(&args);

    EXPECT_EQ(action->invokeCount, 1);
}

TEST(EventTriggerTests, AttachedTriggerInvokesOnMatchingBubblingRoutedEvent)
{
    UIElement element;
    class CountingAction : public TriggerAction {
    public:
        mutable int invokeCount = 0;
        void Invoke(UIElement*) const override {
            ++invokeCount;
        }
    };

    auto action = std::make_shared<CountingAction>();
    auto trigger = std::make_shared<EventTrigger>("MouseEnter");
    trigger->AddAction(action);

    element.AddEventTrigger(trigger);

    NeoWPF::Input::Core::RoutedEventArgs args(NeoWPF::Input::Core::RoutingStrategy::Bubble);
    args.SetRoutedEvent(&NeoWPF::Input::Core::MouseEnterEvent());
    element.RaiseEvent(&args);

    EXPECT_EQ(action->invokeCount, 1);
}

// ============================================================================
// TriggerCollection Tests
// ============================================================================

TEST(TriggerCollectionTests, AddAndRetrieveTriggers)
{
    TriggerCollection collection;
    auto trigger1 = std::make_shared<Trigger>("Property1", 10);
    auto trigger2 = std::make_shared<Trigger>("Property2", 20);

    collection.Add(trigger1);
    collection.Add(trigger2);

    const auto& triggers = collection.GetTriggers();
    EXPECT_EQ(triggers.size(), 2);
    EXPECT_EQ(triggers[0], trigger1);
    EXPECT_EQ(triggers[1], trigger2);
}

TEST(TriggerCollectionTests, AddIgnoresNull)
{
    TriggerCollection collection;
    collection.Add(nullptr);

    EXPECT_EQ(collection.GetTriggers().size(), 0);
}

TEST(TriggerCollectionTests, AddDuplicateIgnored)
{
    TriggerCollection collection;
    auto trigger = std::make_shared<Trigger>("Property", 10);

    collection.Add(trigger);
    collection.Add(trigger);

    EXPECT_EQ(collection.GetTriggers().size(), 1);
}

TEST(TriggerCollectionTests, RemoveTrigger)
{
    TriggerCollection collection;
    auto trigger1 = std::make_shared<Trigger>("Property1", 10);
    auto trigger2 = std::make_shared<Trigger>("Property2", 20);

    collection.Add(trigger1);
    collection.Add(trigger2);

    collection.Remove(trigger1);

    const auto& triggers = collection.GetTriggers();
    EXPECT_EQ(triggers.size(), 1);
    EXPECT_EQ(triggers[0], trigger2);
}

TEST(TriggerCollectionTests, RemoveIgnoresNull)
{
    TriggerCollection collection;
    auto trigger = std::make_shared<Trigger>("Property", 10);
    collection.Add(trigger);

    collection.Remove(nullptr);

    EXPECT_EQ(collection.GetTriggers().size(), 1);
}

TEST(TriggerCollectionTests, RemoveIgnoresNonexistentTrigger)
{
    TriggerCollection collection;
    auto trigger1 = std::make_shared<Trigger>("Property1", 10);
    auto trigger2 = std::make_shared<Trigger>("Property2", 20);
    auto trigger3 = std::make_shared<Trigger>("Property3", 30);

    collection.Add(trigger1);
    collection.Add(trigger2);

    collection.Remove(trigger3);

    EXPECT_EQ(collection.GetTriggers().size(), 2);
}

TEST(TriggerCollectionTests, ClearAllTriggers)
{
    TriggerCollection collection;
    collection.Add(std::make_shared<Trigger>("Property1", 10));
    collection.Add(std::make_shared<Trigger>("Property2", 20));
    collection.Add(std::make_shared<Trigger>("Property3", 30));

    collection.Clear();

    EXPECT_EQ(collection.GetTriggers().size(), 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(TriggerIntegrationTests, TriggerWithSettersIntegration)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);
    element.SetValue(StringTriggerProperty(), std::string("inactive"));

    Trigger trigger("Automated.IntTriggerProperty", 42);
    trigger.AddSetter(Setter("Automated.IntTriggerProperty", 100));
    trigger.AddSetter(Setter("Automated.StringTriggerProperty", std::string("active")));

    EXPECT_TRUE(trigger.Evaluate(&element));

    trigger.Apply(&element);
    EXPECT_EQ(element.GetValue<int>(IntTriggerProperty()), 100);
    EXPECT_EQ(element.GetValue<std::string>(StringTriggerProperty()), "active");
}

TEST(TriggerIntegrationTests, MultipleTriggerChangesProperty)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 0);

    Trigger trigger1("Automated.IntTriggerProperty", 10);
    trigger1.AddSetter(Setter("Automated.IntTriggerProperty", 50));

    Trigger trigger2("Automated.IntTriggerProperty", 20);
    trigger2.AddSetter(Setter("Automated.IntTriggerProperty", 75));

    element.SetValue(IntTriggerProperty(), 10);
    EXPECT_TRUE(trigger1.Evaluate(&element));
    trigger1.Apply(&element);
    EXPECT_EQ(element.GetValue<int>(IntTriggerProperty()), 50);

    element.SetValue(IntTriggerProperty(), 20);
    EXPECT_TRUE(trigger2.Evaluate(&element));
    trigger2.Apply(&element);
    EXPECT_EQ(element.GetValue<int>(IntTriggerProperty()), 75);
}

TEST(TriggerIntegrationTests, CollectionManagementAndEvaluation)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    TriggerCollection collection;
    auto trigger1 = std::make_shared<Trigger>("Automated.IntTriggerProperty", 10);
    auto trigger2 = std::make_shared<Trigger>("Automated.IntTriggerProperty", 42);
    auto trigger3 = std::make_shared<Trigger>("Automated.IntTriggerProperty", 99);

    collection.Add(trigger1);
    collection.Add(trigger2);
    collection.Add(trigger3);

    const auto& triggers = collection.GetTriggers();
    int matchCount = 0;
    for (const auto& t : triggers) {
        if (t->Evaluate(&element)) {
            ++matchCount;
        }
    }

    EXPECT_EQ(matchCount, 1);
    EXPECT_TRUE(trigger2->Evaluate(&element));
}

// ============================================================================
// Concurrency / Race Condition Tests
// ============================================================================

TEST(TriggerConcurrencyTests, ConcurrentCollectionAddRemove)
{
    TriggerCollection collection;
    std::atomic<int> errorCount{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&collection, t, &errorCount]() {
            try {
                for (int i = 0; i < 100; ++i) {
                    auto trigger = std::make_shared<Trigger>(
                        "Property_" + std::to_string(t * 100 + i),
                        i
                    );
                    collection.Add(trigger);

                    if (i % 2 == 0) {
                        collection.Remove(trigger);
                    }
                }
            } catch (...) {
                ++errorCount;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(errorCount, 0) << "Concurrent Add/Remove should not throw";
    EXPECT_GT(collection.GetTriggers().size(), 0);
}

TEST(TriggerConcurrencyTests, ConcurrentTriggerEvaluation)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 42);

    std::vector<std::shared_ptr<Trigger>> triggers;
    for (int i = 0; i < 100; ++i) {
        triggers.push_back(std::make_shared<Trigger>("Automated.IntTriggerProperty", 42));
    }

    std::atomic<int> evaluationCount{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < 8; ++t) {
        threads.emplace_back([&triggers, &element, &evaluationCount]() {
            for (const auto& trigger : triggers) {
                if (trigger->Evaluate(&element)) {
                    ++evaluationCount;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(evaluationCount, 100 * 8);
}

TEST(TriggerConcurrencyTests, ConcurrentSetterApplication)
{
    UIElement element;
    element.SetValue(IntTriggerProperty(), 0);

    std::vector<std::shared_ptr<Trigger>> triggers;
    for (int i = 0; i < 50; ++i) {
        auto trigger = std::make_shared<Trigger>(
            "Automated.IntTriggerProperty",
            i
        );
        trigger->AddSetter(Setter("Automated.IntTriggerProperty", i * 10));
        triggers.push_back(trigger);
    }

    std::atomic<int> errorCount{0};
    std::vector<std::thread> threads;

    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&triggers, &element, &errorCount]() {
            try {
                for (const auto& trigger : triggers) {
                    trigger->Apply(&element);
                }
            } catch (...) {
                ++errorCount;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(errorCount, 0) << "Concurrent Setter application should not throw";
    const int finalValue = element.GetValue<int>(IntTriggerProperty());
    EXPECT_GE(finalValue, 0);
    EXPECT_LE(finalValue, 490);
}

TEST(TriggerConcurrencyTests, ConcurrentTriggerCollectionOperations)
{
    TriggerCollection collection;
    std::atomic<int> addCount{0};
    std::atomic<int> removeCount{0};
    std::atomic<int> errorCount{0};
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Trigger>> sharedTriggers;

    for (int i = 0; i < 20; ++i) {
        sharedTriggers.push_back(
            std::make_shared<Trigger>("Property_" + std::to_string(i), i)
        );
    }

    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&]() {
            try {
                for (int i = 0; i < 50; ++i) {
                    if (i % 3 == 0) {
                        auto trigger = std::make_shared<Trigger>(
                            "TempProperty_" + std::to_string(t * 50 + i),
                            i
                        );
                        collection.Add(trigger);
                        ++addCount;
                    } else if (i % 3 == 1 && !sharedTriggers.empty()) {
                        collection.Remove(sharedTriggers[i % sharedTriggers.size()]);
                        ++removeCount;
                    } else {
                        const auto& triggers = collection.GetTriggers();
                        (void)triggers.size(); 
                    }
                }
            } catch (...) {
                ++errorCount;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(errorCount, 0) << "Concurrent collection operations should not throw";
    EXPECT_GT(addCount, 0) << "Should have added some triggers";
    // Final collection state should be valid
    const auto& finalTriggers = collection.GetTriggers();
    EXPECT_GE(finalTriggers.size(), 0);
}

