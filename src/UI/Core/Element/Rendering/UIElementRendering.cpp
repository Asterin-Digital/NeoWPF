#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <vector>

namespace NeoWPF::UI {

void UIElement::OnInitialize()
{
    if (!m_hasTemplateApplied && m_template) {
        ApplyTemplate();
    }

    if (m_appliedTemplateRoot) {
        m_appliedTemplateRoot->Initialize();
    }
}

void UIElement::BeginAnimation(float from, float to, float durationSeconds) {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Opacity", *this);
    if (!property) {
        return;
    }
    BeginAnimation(*property, from, to, durationSeconds);
}

std::shared_ptr<NeoWPF::UI::Core::Transform> UIElement::GetRenderTransform() const noexcept {
    return m_renderTransform;
}

void UIElement::SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept {
    if (auto oldScaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(m_renderTransform)) {
        oldScaleTransform->m_invalidateCallback = nullptr;
    }
    if (auto oldRotateTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::RotateTransform>(m_renderTransform)) {
        oldRotateTransform->m_invalidateCallback = nullptr;
    }

    m_renderTransform = transform;
    if (auto scaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(transform)) {
        scaleTransform->m_invalidateCallback = [this]() {
            this->InvalidateVisual();
        };
    }
    if (auto rotateTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::RotateTransform>(transform)) {
        rotateTransform->m_invalidateCallback = [this]() {
            this->InvalidateVisual();
        };
    }
    InvalidateVisual();
}

NeoWPF::UI::Core::Point UIElement::GetRenderTransformOrigin() const noexcept {
    return m_renderTransformOrigin;
}

void UIElement::SetRenderTransformOrigin(NeoWPF::UI::Core::Point origin) noexcept {
    m_renderTransformOrigin = origin;
    InvalidateVisual();
}

void UIElement::SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr) {
    if (m_appliedTemplateRoot) {
        m_appliedTemplateRoot = nullptr;
    }

    m_template = templatePtr;
    m_hasTemplateApplied = false;
}

bool UIElement::ApplyTemplate() {
    if (!m_template || m_hasTemplateApplied) {
        return false;
    }

    m_template->Apply(this);
    m_appliedTemplateRoot = m_template->GetVisualTreeRoot();
    m_hasTemplateApplied = true;
    return true;
}

void UIElement::ClearTemplateVisualRoot() noexcept {
    m_appliedTemplateRoot = nullptr;
}

std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> UIElement::GetTemplateVisualStateManager() const {
    if (!m_template) {
        return nullptr;
    }

    return m_template->GetVisualStateManager();
}

void UIElement::RegisterTemplateChild(const std::string& name, const std::shared_ptr<Visual>& child) {
    if (name.empty() || !child) return;
    m_templateChildren[name] = child;
}

std::shared_ptr<Visual> UIElement::FindTemplateChild(const std::string& name) const {
    if (name.empty()) return nullptr;
    auto it = m_templateChildren.find(name);
    return it != m_templateChildren.end() ? it->second : nullptr;
}

bool UIElement::HasTemplateChild(const std::string& name) const {
    if (name.empty()) return false;
    return m_templateChildren.find(name) != m_templateChildren.end();
}

void UIElement::ClearTemplateChildren() noexcept {
    m_templateChildren.clear();
    m_hasTemplateApplied = false;
}

bool UIElement::SetStyleProperty(const std::string& name, uint32_t value)
{
    if (name.empty()) {
        return false;
    }

    auto property = Properties::DependencyPropertyRegistry::Get().FindProperty(name, *this);
    if (!property) {
        return false;
    }

    SetValueInternal(*property, std::any(value), Properties::PropertyValueSource::Style);
    return true;
}

bool UIElement::SetStylePropertyValue(const std::string& name, const std::any& value)
{
    if (name.empty()) {
        return false;
    }

    auto property = Properties::DependencyPropertyRegistry::Get().FindProperty(name, *this);
    if (!property) {
        return false;
    }

    SetValueInternal(*property, value, Properties::PropertyValueSource::Style);
    return true;
}

bool UIElement::SetPropertyValue(const std::string& name, const std::any& value)
{
    if (name.empty()) {
        return false;
    }

    auto property = Properties::DependencyPropertyRegistry::Get().FindProperty(name, *this);
    if (!property) {
        return false;
    }

    SetValueInternal(*property, value, Properties::PropertyValueSource::Local);
    PropertyChanged.Invoke(name, value);
    return true;
}

bool UIElement::SetResource(const std::string& key, const std::any& value)
{
    if (key.empty()) {
        return false;
    }

    m_resources[key] = value;
    return true;
}

bool UIElement::TryFindResource(const std::string& key, std::any& outValue) const
{
    if (key.empty()) {
        return false;
    }

    const UIElement* current = this;
    while (current)
    {
        const auto it = current->m_resources.find(key);
        if (it != current->m_resources.end()) {
            outValue = it->second;
            return true;
        }

        const auto* parentVisual = current->GetParent();
        current = dynamic_cast<const UIElement*>(parentVisual);
    }

    return NeoWPF::Core::ApplicationResources::Resources::TryFindResource(key, outValue);
}

std::any UIElement::FindResource(const std::string& key) const
{
    std::any value;
    if (TryFindResource(key, value)) {
        return value;
    }
    return {};
}

std::any UIElement::GetPropertyValue(const std::string& name) const
{
    if (name.empty()) {
        return {};
    }

    auto property = Properties::DependencyPropertyRegistry::Get().FindProperty(name, *this);
    if (!property) {
        return {};
    }

    return GetRawValue(*property);
}


void UIElement::AddHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent,
                           const NeoWPF::Input::Core::RoutedEventHandler& handler)
{
    m_routedEventHandlers[routedEvent.EventTypeId] += [handler](UIElement* sender, NeoWPF::Input::Core::RoutedEventArgs* args) {
        handler(sender, args);
    };
}

void UIElement::RaiseEvent(NeoWPF::Input::Core::RoutedEventArgs* args)
{
    if (!args || !args->GetRoutedEvent()) {
        return;
    }

    if (!args->GetOriginalSource()) {
        args->SetOriginalSource(this);
    }

    if (!args->GetSource()) {
        args->SetSource(this);
    }

    const auto routingStrategy = args->GetRoutingStrategy();
    const auto eventTypeId = args->GetRoutedEvent()->EventTypeId;

    auto invokeOnElement = [&](UIElement* element) {
        if (!element) {
            return;
        }
        args->SetSource(element);
        auto it = element->m_routedEventHandlers.find(eventTypeId);
        if (it != element->m_routedEventHandlers.end())
        {
            it->second.Invoke(element, args);
        }
    };

    if (routingStrategy == NeoWPF::Input::Core::RoutingStrategy::Direct)
    {
        invokeOnElement(this);
        return;
    }

    if (routingStrategy == NeoWPF::Input::Core::RoutingStrategy::Tunnel)
    {
        std::vector<UIElement*> route;
        auto* current = this;
        while (current)
        {
            route.push_back(current);
            auto* parentVisual = current->GetParent();
            current = dynamic_cast<UIElement*>(parentVisual);
        }
        for (auto it = route.rbegin(); it != route.rend() && !args->IsHandled(); ++it)
        {
            invokeOnElement(*it);
        }
        return;
    }

    if (routingStrategy == NeoWPF::Input::Core::RoutingStrategy::Bubble)
    {
        auto* current = this;
        while (current && !args->IsHandled())
        {
            invokeOnElement(current);
            auto* parentVisual = current->GetParent();
            current = dynamic_cast<UIElement*>(parentVisual);
        }
    }
}

}
