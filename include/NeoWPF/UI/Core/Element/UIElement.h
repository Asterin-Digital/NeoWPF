#pragma once

#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/EventHandler.h>
#include <NeoWPF/UI/Properties/Property/PropertyValue.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <any>
#include <vector>

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
    struct PropertyInfo;
    class PropertyRegistry;
}

namespace NeoWPF::UI {

class UIElement : public Visual {
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
    bool IsDirty() const noexcept { return m_isDirty; }
    void ClearDirty() noexcept;

    std::shared_ptr<NeoWPF::UI::Core::Transform> GetRenderTransform() const noexcept;
    void SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept;

    NeoWPF::UI::Core::Point GetRenderTransformOrigin() const noexcept;
    void SetRenderTransformOrigin(NeoWPF::UI::Core::Point origin) noexcept;

    void BeginAnimation(float from, float to, float durationSeconds);

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

    void SetProperty(const std::string& name, uint32_t value);
    bool TryGetProperty(const std::string& name, uint32_t& outValue) const;
    void SetStyleProperty(const std::string& name, uint32_t value);
    bool SetStylePropertyValue(const std::string& name, const std::any& value);

    bool SetPropertyValue(const std::string& name, const std::any& value);
    std::any GetPropertyValue(const std::string& name) const;
    std::vector<Properties::PropertyInfo> GetPropertyInfo() const;

    NeoWPF::Core::Event<std::string, std::any> PropertyChanged;

    void AddHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                    const NeoWPF::Input::Core::RoutedEventHandler& handler);
    void RaiseEvent(NeoWPF::Input::Core::RoutedEventArgs* args);

    virtual void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) {}
    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) {}
    virtual void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) {}
    virtual void OnGotFocus() {}
    virtual void OnLostFocus() {}

protected:
    bool m_isEnabled = true;
    bool IsPropertyLocal(const std::string& name) const;
    void SetPropertySource(const std::string& name, Properties::PropertySource source);

private:
    friend class Properties::PropertyRegistry;
    bool m_isVisible = true;
    bool m_isDirty = true;
    float m_opacity = 1.0f;
    bool m_hasTemplateApplied = false;
    std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase> m_template;
    std::unordered_map<std::string, std::shared_ptr<Visual>> m_templateChildren;
    std::unordered_map<std::string, uint32_t> m_properties;
    std::unordered_map<std::string, NeoWPF::UI::Properties::PropertySource> m_propertySources;
    std::shared_ptr<Visual> m_appliedTemplateRoot;
    std::unordered_map<std::size_t, NeoWPF::Core::Event<UIElement*, NeoWPF::Input::Core::RoutedEventArgs*>> m_routedEventHandlers;
    std::shared_ptr<NeoWPF::UI::Core::Transform> m_renderTransform;
    NeoWPF::UI::Core::Point m_renderTransformOrigin{0.5f, 0.5f};
};

}
