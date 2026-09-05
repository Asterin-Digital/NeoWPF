#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/EventHandler.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Definitions/DoubleAnimation.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Styling/Triggers/EventTrigger.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/IXamlResourceHost.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <type_traits>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
    class KeyboardEventArgs;
    class RoutedEventArgs;
    struct RoutedEvent;
}

namespace NeoWPF::UI::Core {
    class Transform;
    struct Point {
        float X;
        float Y;
        Point(float x = 0.0f, float y = 0.0f) : X(x), Y(y) {}
    };
}

namespace NeoWPF::Input {
    void FocusElement(NeoWPF::UI::UIElement* element);
}

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Templates {
class ControlTemplateBase;
}

namespace NeoWPF::UI::Styling {
class VisualStateManager;
}

namespace NeoWPF::UI::Properties {
    class DependencyProperty;
    class DependencyPropertyRegistry;
    struct PropertyMetadata;
}

namespace NeoWPF::UI::Styling::Triggers {
class EventTrigger;
}

namespace NeoWPF::UI {

class UIElement : public Visual,
                  public Properties::DependencyObject,
                  public NeoWPF::UI::Markup::Xaml::IXamlResourceHost {
public:
    static const Properties::DependencyProperty& VisibilityProperty();
    static const Properties::DependencyProperty& IsEnabledProperty();
    static const Properties::DependencyProperty& IsTabStopProperty();
    static const Properties::DependencyProperty& TabIndexProperty();
    static const Properties::DependencyProperty& IsFocusedProperty();
    static const Properties::DependencyProperty& OpacityProperty();

    /*
        @summary
        Returns whether the element is currently visible to the user.
    */
    bool IsVisible() const noexcept { return m_isVisible; }

    /*
        @summary
        Sets the element's visibility state.

        @param visible
        True to show the element, false to hide it.
    */
    void SetVisibility(bool visible) noexcept {
        if (m_isVisible != visible) {
            m_isVisible = visible;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Convenience alias for SetVisibility.
    */
    void SetVisible(bool visible) noexcept { SetVisibility(visible); }

    /*
        @summary
        Returns whether the element currently accepts user interaction.
    */
    bool IsEnabled() const noexcept { return m_isEnabled; }

    /*
        @summary
        Enables or disables the element for interaction.

        @param enabled
        True to enable input handling, false to disable it.
    */
    void SetEnabled(bool enabled) noexcept { m_isEnabled = enabled; }

    /*
        @summary
        Returns whether the element participates in keyboard tab navigation.
    */
    bool IsTabStop() const noexcept { return m_isTabStop; }

    /*
        @summary
        Sets whether the element can receive focus via tab navigation.

        @param tabStop
        True to allow tab focus, false otherwise.
    */
    void SetTabStop(bool tabStop) noexcept {
        if (m_isTabStop != tabStop) {
            m_isTabStop = tabStop;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns the tab index used for keyboard navigation order.
    */
    int GetTabIndex() const noexcept { return m_tabIndex; }

    /*
        @summary
        Sets the tab index used for focus ordering.

        @param tabIndex
        The desired navigation index.
    */
    void SetTabIndex(int tabIndex) noexcept {
        if (m_tabIndex != tabIndex) {
            m_tabIndex = tabIndex;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns whether the element currently owns input focus.
    */
    bool IsFocused() const noexcept { return m_isFocused; }

    /*
        @summary
        Sets the focused state of the element.

        @param focused
        True when the element has keyboard focus.
    */
    void SetFocused(bool focused) noexcept {
        if (m_isFocused != focused) {
            m_isFocused = focused;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns whether the element can receive keyboard focus under the current state.
    */
    bool CanFocus() const noexcept { return IsEnabled() && IsVisible() && IsTabStop(); }

    /*
        @summary
        Attempts to focus the element.

        @return
        True if the element accepted focus, false otherwise.
    */
    bool Focus() noexcept {
        if (!CanFocus()) {
            return false;
        }

        NeoWPF::Input::FocusElement(this);
        return IsFocused();
    }

    /*
        @summary
        Returns the element opacity value.
    */
    float GetOpacity() const noexcept { return m_opacity; }

    /*
        @summary
        Sets the opacity using a clamped value between 0 and 1.

        @param opacity
        The target opacity.
    */
    void SetOpacity(float opacity) noexcept {
        if (opacity < 0.0f) {
            opacity = 0.0f;
        } else if (opacity > 1.0f) {
            opacity = 1.0f;
        }
        if (m_opacity != opacity) {
            m_opacity = opacity;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Marks the element as needing a redraw.

        @internal
    */
    void InvalidateVisual();

    /*
        @summary
        Invalidates layout measurement for the element.

        @internal
    */
    virtual void InvalidateMeasure();

    /*
        @summary
        Invalidates layout arrangement for the element.

        @internal
    */
    virtual void InvalidateArrange();

    /*
        @summary
        Returns whether the element currently has pending invalidation.

        @internal
    */
    bool IsDirty() const noexcept { return m_isDirty; }

    /*
        @summary
        Clears the dirty flag after visual or layout work has completed.

        @internal
    */
    void ClearDirty() noexcept;

    /*
        @summary
        Returns the transform used to render the element.
    */
    std::shared_ptr<NeoWPF::UI::Core::Transform> GetRenderTransform() const noexcept;

    /*
        @summary
        Sets the render transform applied to the element during painting.

        @param transform
        The transform to use for rendering.
    */
    void SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept;

    /*
        @summary
        Returns the origin used for render transform operations.
    */
    NeoWPF::UI::Core::Point GetRenderTransformOrigin() const noexcept;

    /*
        @summary
        Sets the render transform origin relative to the element bounds.

        @param origin
        The local transform origin.
    */
    void SetRenderTransformOrigin(NeoWPF::UI::Core::Point origin) noexcept;

    /*
        @summary
        Returns the local transform matrix for the element.

        @internal
    */
    virtual NeoWPF::UI::Core::Matrix4 GetLocalTransformMatrix() const noexcept;

    /*
        @summary
        Returns the transform of the element in world coordinates.
    */
    NeoWPF::UI::Core::Matrix4 GetWorldTransformMatrix() const noexcept;

    /*
        @summary
        Returns the element's accumulated rotation in radians.
    */
    float GetWorldRotationRadians() const noexcept;

    /*
        @summary
        Begins a simple animation from one float value to another.

        @param from
        The starting value.

        @param to
        The ending value.

        @param durationSeconds
        The animation duration in seconds.
    */
    void BeginAnimation(float from, float to, float durationSeconds);

    template<typename T>
    void BeginAnimation(const Properties::DependencyProperty& property,
                        const T& fromValue,
                        const T& toValue,
                        float durationSeconds) {
        if (durationSeconds <= 0.0f) {
            SetValue(property, std::any(toValue));
            return;
        }

        if constexpr (std::is_same_v<T, float>) {
            using namespace NeoWPF::UI::Animation;
            auto storyboard = std::make_shared<Storyboard>();
            auto animation = std::make_shared<DoubleAnimation>();
            animation->SetFrom(fromValue);
            animation->SetTo(toValue);
            animation->SetDuration(durationSeconds);
            storyboard->AddChild({}, property.GetName(), animation);
            NeoWPF::UI::BeginStoryboard(storyboard, this);
        } else {
            SetValue(property, std::any(toValue));
        }
    }

    template<typename T>
    void BeginAnimation(const Properties::DependencyProperty& property,
                        const T& toValue,
                        float durationSeconds) {
        if (durationSeconds <= 0.0f) {
            SetValue(property, std::any(toValue));
            return;
        }

        if (property.GetPropertyType() == typeid(T)) {
            const auto currentValue = GetValue<T>(property);
            BeginAnimation(property, currentValue, toValue, durationSeconds);
        }
    }

    template<typename T>
    void BeginAnimation(const std::string& propertyName,
                        const T& fromValue,
                        const T& toValue,
                        float durationSeconds) {
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty(propertyName, *this);
        if (!property) {
            return;
        }
        BeginAnimation(*property, fromValue, toValue, durationSeconds);
    }

    template<typename T>
    void BeginAnimation(const std::string& propertyName,
                        const T& toValue,
                        float durationSeconds) {
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty(propertyName, *this);
        if (!property) {
            return;
        }
        BeginAnimation(*property, toValue, durationSeconds);
    }

    /*
        @summary
        Performs measure logic for the element.

        @param availableWidth
        The maximum width available during measurement.

        @param availableHeight
        The maximum height available during measurement.

        @internal
    */
    virtual void Measure(int availableWidth, int availableHeight) {}

    /*
        @summary
        Performs arrange logic for the element.

        @param finalWidth
        The width assigned during layout.

        @param finalHeight
        The height assigned during layout.

        @internal
    */
    virtual void Arrange(int finalWidth, int finalHeight) {}

    /*
        @summary
        Returns whether the element is eligible for hit testing.
    */
    virtual bool IsHitTestVisible() const noexcept { return true; }

    /*
        @summary
        Returns whether the element can participate in user interaction.
    */
    virtual bool IsInteractable() const noexcept { return false; }

    /*
        @summary
        Returns whether the element has any routed event handlers for the given routed event.

        @param routedEvent
        The event to inspect.
    */
    bool HasRoutedEventHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent) const;

    /*
        @summary
        Applies a control template to the element.

        @param templatePtr
        The template to assign.
    */
    void SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr);

    /*
        @summary
        Returns the active control template, if any.
    */
    const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& GetTemplate() const noexcept { return m_template; }

    /*
        @summary
        Returns whether this element currently has an active template.
    */
    bool HasTemplate() const noexcept { return static_cast<bool>(m_template); }

    /*
        @summary
        Applies the current template to the element tree.

        @return
        True if template application succeeded.
    */
    bool ApplyTemplate();

    /*
        @summary
        Clears any visual root assigned by the template.

        @internal
    */
    void ClearTemplateVisualRoot() noexcept;

    /*
        @summary
        Returns the visual state manager associated with the template.
    */
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetTemplateVisualStateManager() const;

    /*
        @summary
        Registers a named child from a control template.

        @param name
        The name used to identify the template child.

        @param child
        The visual instance to register.
    */
    void RegisterTemplateChild(const std::string& name, const std::shared_ptr<Visual>& child);

    /*
        @summary
        Finds a template child by name.

        @param name
        The template child name to look up.

        @return
        The matching child visual, or nullptr if not found.
    */
    std::shared_ptr<Visual> FindTemplateChild(const std::string& name) const;

    /*
        @summary
        Returns whether a template child with the given name exists.
    */
    bool HasTemplateChild(const std::string& name) const;

    /*
        @summary
        Returns all registered template child names.
    */
    std::vector<std::string> GetTemplateChildNames() const;

    /*
        @summary
        Removes all template children from the element.

        @internal
    */
    void ClearTemplateChildren() noexcept;

    /*
        @summary
        Sets a style property by name using an unsigned integer value.

        @param name
        The property name.

        @param value
        The property value.
    */
    bool SetStyleProperty(const std::string& name, uint32_t value);

    /*
        @summary
        Sets a style property by name to an arbitrary value.

        @param name
        The property name.

        @param value
        The property value.
    */
    bool SetStylePropertyValue(const std::string& name, const std::any& value);

    /*
        @summary
        Sets a property value by name.

        @param name
        The property name.

        @param value
        The value to assign.
    */
    bool SetPropertyValue(const std::string& name, const std::any& value);

    /*
        @summary
        Gets a property value by name.

        @param name
        The property name.

        @return
        The property value, if one exists.
    */
    std::any GetPropertyValue(const std::string& name) const;

    /*
        @summary
        Stores a named resource value on the element.

        @param key
        The resource key.

        @param value
        The resource value.
    */
    bool SetResource(const std::string& key, const std::any& value);
    bool SetXamlResource(const std::string& key, const std::any& value) override {
        return SetResource(key, value);
    }

    /*
        @summary
        Tries to lookup a named resource value on the element.

        @param key
        The resource key.

        @param outValue
        The value if found.

        @return
        True when a matching resource exists.
    */
    bool TryFindResource(const std::string& key, std::any& outValue) const;
    bool TryFindXamlResource(const std::string& key, std::any& outValue) const override {
        return TryFindResource(key, outValue);
    }

    /*
        @summary
        Finds a named resource on the element.

        @param key
        The resource key.

        @return
        The stored resource value, or an empty any if none exists.
    */
    std::any FindResource(const std::string& key) const;

    template<typename T>
    T GetResourceValue(const std::string& key, const T& fallbackValue) const {
        std::any value;
        if (TryFindResource(key, value)) {
            if (const auto* typedValue = std::any_cast<T>(&value)) {
                return *typedValue;
            }
        }
        return fallbackValue;
    }

    /*
        @summary
        Raised when a dependency property changes value.
    */
    NeoWPF::Core::Event<std::string, std::any> PropertyChanged;

    /*
        @summary
        Adds a routed event handler to the element.

        @param routedEvent
        The routed event to handle.

        @param handler
        The callback invoked when the event is raised.
    */
    void AddHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                    const NeoWPF::Input::Core::RoutedEventHandler& handler);

    /*
        @summary
        Adds an event trigger to the element.

        @param trigger
        The event trigger to associate with this element.
    */
    void AddEventTrigger(const std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>& trigger);

    /*
        @summary
        Removes an event trigger from the element.

        @param trigger
        The event trigger to detach.
    */
    void RemoveEventTrigger(const std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>& trigger);

    /*
        @summary
        Returns the event triggers attached to this element.
    */
    std::vector<std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>> GetEventTriggers() const;

    /*
        @summary
        Raises a routed event on the element.

        @param args
        The routed event arguments for the raised event.
    */
    void RaiseEvent(NeoWPF::Input::Core::RoutedEventArgs* args);

    /*
        @summary
        Called when a dependency property has changed value.

        @param property
        The property that changed.

        @param metadata
        The property metadata.

        @param oldValue
        The previous value.

        @param newValue
        The updated value.

        @param source
        The source of the property value.
    */
    virtual void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                             const Properties::PropertyMetadata& metadata,
                                             const std::any& oldValue,
                                             const std::any& newValue,
                                             Properties::PropertyValueSource source);

    /*
        @summary
        Called when mouse input is received.

        @param args
        The mouse event arguments.
    */
    virtual void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) {}

    /*
        @summary
        Called when the mouse button is pressed.

        @param x
        The local x-coordinate.

        @param y
        The local y-coordinate.

        @param buttonState
        The button state mask for the input.
    */
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) {}

    /*
        @summary
        Called when the mouse button is released.

        @param x
        The local x-coordinate.

        @param y
        The local y-coordinate.

        @param buttonState
        The button state mask for the input.
    */
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) {}

    /*
        @summary
        Called when the mouse pointer enters the element.

        @param args
        The mouse event arguments.
    */
    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) {}

    /*
        @summary
        Called when the mouse pointer leaves the element.

        @param args
        The mouse event arguments.
    */
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) {}

    /*
        @summary
        Called when keyboard input is received.

        @param args
        The keyboard event arguments.
    */
    virtual void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) {}

    /*
        @summary
        Called when the element gains focus.
    */
    virtual void OnGotFocus();

    /*
        @summary
        Called when the element loses focus.
    */
    virtual void OnLostFocus();

protected:
    void OnInitialize() override;

    void InvalidateTransformCache() const noexcept;
    void InvalidateWorldTransformCache() const noexcept;

    template<typename T>
    void ApplyAnimatedValue(const Properties::DependencyProperty& property,
                            const T& fromValue,
                            const T& toValue,
                            float progress) {
        if constexpr (std::is_same_v<T, NeoWPF::UI::Color>) {
            const auto fromArgb = static_cast<float>(fromValue.ToArgb());
            const auto toArgb = static_cast<float>(toValue.ToArgb());
            const auto currentArgb = std::lround(fromArgb + (toArgb - fromArgb) * progress);
            SetValue(property, std::any(NeoWPF::UI::Color::FromArgb(static_cast<std::uint32_t>(currentArgb))));
        } else if constexpr (std::is_same_v<T, NeoWPF::UI::CornerRadius>) {
            const auto fromTopLeft = fromValue.TopLeft + (toValue.TopLeft - fromValue.TopLeft) * progress;
            const auto fromTopRight = fromValue.TopRight + (toValue.TopRight - fromValue.TopRight) * progress;
            const auto fromBottomRight = fromValue.BottomRight + (toValue.BottomRight - fromValue.BottomRight) * progress;
            const auto fromBottomLeft = fromValue.BottomLeft + (toValue.BottomLeft - fromValue.BottomLeft) * progress;
            SetValue(property, std::any(NeoWPF::UI::CornerRadius(fromTopLeft, fromTopRight, fromBottomRight, fromBottomLeft)));
        } else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            const auto fromInt = static_cast<double>(fromValue);
            const auto toInt = static_cast<double>(toValue);
            const auto currentInt = std::lround(fromInt + (toInt - fromInt) * progress);
            SetValue(property, std::any(static_cast<T>(currentInt)));
        } else if constexpr (std::is_floating_point_v<T>) {
            const auto currentFloat = fromValue + (toValue - fromValue) * progress;
            SetValue(property, std::any(currentFloat));
        } else if constexpr (std::is_enum_v<T>) {
            using Underlying = std::underlying_type_t<T>;
            const auto fromEnum = static_cast<double>(static_cast<Underlying>(fromValue));
            const auto toEnum = static_cast<double>(static_cast<Underlying>(toValue));
            const auto currentEnum = std::lround(fromEnum + (toEnum - fromEnum) * progress);
            SetValue(property, std::any(static_cast<T>(static_cast<Underlying>(currentEnum))));
        } else {
            SetValue(property, std::any(toValue));
        }
    }

    bool m_isEnabled = true;
    bool m_isTabStop = true;
    int m_tabIndex = 0;
    bool m_isFocused = false;

private:
    bool m_isVisible = true;
    bool m_isDirty = true;
    float m_opacity = 1.0f;
    bool m_hasTemplateApplied = false;
    std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase> m_template;
    std::unordered_map<std::string, std::shared_ptr<Visual>> m_templateChildren;
    std::shared_ptr<Visual> m_appliedTemplateRoot;
    std::unordered_map<std::size_t, NeoWPF::Core::Event<UIElement*, NeoWPF::Input::Core::RoutedEventArgs*>> m_routedEventHandlers;
    std::shared_ptr<NeoWPF::UI::Core::Transform> m_renderTransform;
    NeoWPF::UI::Core::Point m_renderTransformOrigin{0.5f, 0.5f};
    mutable bool m_localTransformDirty = true;
    mutable bool m_worldTransformDirty = true;
    mutable NeoWPF::UI::Core::Matrix4 m_cachedLocalTransform;
    mutable NeoWPF::UI::Core::Matrix4 m_cachedWorldTransform;
    std::unordered_map<std::string, std::any> m_resources;
    std::vector<std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>> m_eventTriggers;
};

}
