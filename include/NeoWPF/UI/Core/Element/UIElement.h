#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/EventHandler.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/Core/Application/ApplicationResources.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
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

namespace NeoWPF::UI {

class UIElement : public Visual, public Properties::DependencyObject {
public:
    bool IsVisible() const noexcept { return m_isVisible; }
    void SetVisibility(bool visible) noexcept {
        if (m_isVisible != visible) {
            m_isVisible = visible;
            InvalidateVisual();
        }
    }

    bool IsEnabled() const noexcept { return m_isEnabled; }
    void SetEnabled(bool enabled) noexcept { m_isEnabled = enabled; }

    float GetOpacity() const noexcept { return m_opacity; }
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

    void InvalidateVisual();
    virtual void InvalidateMeasure();
    virtual void InvalidateArrange();
    bool IsDirty() const noexcept { return m_isDirty; }
    void ClearDirty() noexcept;

    std::shared_ptr<NeoWPF::UI::Core::Transform> GetRenderTransform() const noexcept;
    void SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept;

    NeoWPF::UI::Core::Point GetRenderTransformOrigin() const noexcept;
    void SetRenderTransformOrigin(NeoWPF::UI::Core::Point origin) noexcept;

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

        using namespace NeoWPF::UI::Animation;
        auto storyboard = std::make_shared<Storyboard>();
        auto timeline = std::make_shared<Timeline>(0.0f, 1.0f, durationSeconds, [this, property, fromValue, toValue](float progress) {
            this->ApplyAnimatedValue(property, fromValue, toValue, progress);
        });
        storyboard->AddTimeline(timeline);
        NeoWPF::UI::AddStoryboard(storyboard);
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
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty(propertyName, typeid(*this));
        if (!property) {
            return;
        }
        BeginAnimation(*property, fromValue, toValue, durationSeconds);
    }

    template<typename T>
    void BeginAnimation(const std::string& propertyName,
                        const T& toValue,
                        float durationSeconds) {
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty(propertyName, typeid(*this));
        if (!property) {
            return;
        }
        BeginAnimation(*property, toValue, durationSeconds);
    }

    virtual void Measure(int availableWidth, int availableHeight) {}
    virtual void Arrange(int finalWidth, int finalHeight) {}

    virtual bool IsHitTestVisible() const noexcept { return true; }
    virtual bool IsInteractable() const noexcept { return true; }

    void SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr);
    const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& GetTemplate() const noexcept { return m_template; }

    bool HasTemplate() const noexcept { return static_cast<bool>(m_template); }

    bool ApplyTemplate();
    void ClearTemplateVisualRoot() noexcept;

    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetTemplateVisualStateManager() const;

    void RegisterTemplateChild(const std::string& name, const std::shared_ptr<Visual>& child);
    std::shared_ptr<Visual> FindTemplateChild(const std::string& name) const;
    bool HasTemplateChild(const std::string& name) const;
    void ClearTemplateChildren() noexcept;
    bool SetStyleProperty(const std::string& name, uint32_t value);
    bool SetStylePropertyValue(const std::string& name, const std::any& value);
    bool SetPropertyValue(const std::string& name, const std::any& value);
    std::any GetPropertyValue(const std::string& name) const;

    bool SetResource(const std::string& key, const std::any& value);
    bool TryFindResource(const std::string& key, std::any& outValue) const;
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

    NeoWPF::Core::Event<std::string, std::any> PropertyChanged;

    void AddHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                    const NeoWPF::Input::Core::RoutedEventHandler& handler);
    void RaiseEvent(NeoWPF::Input::Core::RoutedEventArgs* args);

    virtual void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                             const Properties::PropertyMetadata& metadata,
                                             const std::any& oldValue,
                                             const std::any& newValue,
                                             Properties::PropertyValueSource source);

    virtual void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) {}
    virtual void OnGotFocus() {}
    virtual void OnLostFocus() {}

protected:
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
    std::unordered_map<std::string, std::any> m_resources;
};

}
