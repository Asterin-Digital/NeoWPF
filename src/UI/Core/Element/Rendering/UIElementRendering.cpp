#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <cmath>
#include <vector>

void NeoWPF::UI::UIElement::OnGotFocus()
{
    if (auto manager = GetTemplateVisualStateManager()) {
        if (manager->FindState("Focused")) {
            manager->GoToState(this, "Focused", false);
        }
    }
    InvalidateVisual();
}

void NeoWPF::UI::UIElement::OnLostFocus()
{
    if (auto manager = GetTemplateVisualStateManager()) {
        if (manager->FindState("Normal")) {
            manager->GoToState(this, "Normal", false);
        }
    }
    InvalidateVisual();
}

namespace NeoWPF::UI {

namespace {

}

void UIElement::OnInitialize()
{
    const auto styleValue = GetRawValue(StyleProperty());
    const auto* style = std::any_cast<std::shared_ptr<NeoWPF::UI::Styling::Style>>(&styleValue);
    if (style && *style) {
        (*style)->ApplyTo(*this);
    } else {
        std::shared_ptr<NeoWPF::UI::Styling::Style> implicitStyle;
        const auto targetType = std::type_index(typeid(*this));
        const UIElement* current = this;
        while (current && !current->TryFindImplicitStyle(targetType, implicitStyle)) {
            current = dynamic_cast<const UIElement*>(current->GetParent());
        }

        if (!implicitStyle) {
            NeoWPF::Core::ApplicationResources::Resources::TryFindImplicitStyle(
                targetType, implicitStyle);
        }

        if (implicitStyle) {
            implicitStyle->ApplyTo(*this);
        }
    }

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

void UIElement::InvalidateTransformCache() const noexcept {
    m_localTransformDirty = true;
    InvalidateWorldTransformCache();
}

void UIElement::InvalidateWorldTransformCache() const noexcept {
    if (m_worldTransformDirty) {
        return;
    }

    m_worldTransformDirty = true;
    for (auto* child : GetVisualChildren()) {
        if (auto childElement = dynamic_cast<UIElement*>(child)) {
            childElement->InvalidateWorldTransformCache();
        }
    }
}

NeoWPF::UI::Core::Matrix4 UIElement::GetLocalTransformMatrix() const noexcept {
    if (m_localTransformDirty) {
        m_cachedLocalTransform = NeoWPF::UI::Core::Matrix4::Identity();
        if (auto transform = GetRenderTransform()) {
            const auto origin = GetRenderTransformOrigin();
            m_cachedLocalTransform = NeoWPF::UI::Core::Matrix4::Translation(-origin.X, -origin.Y)
                * transform->GetMatrix()
                * NeoWPF::UI::Core::Matrix4::Translation(origin.X, origin.Y);
        }
        m_localTransformDirty = false;
    }
    return m_cachedLocalTransform;
}

NeoWPF::UI::Core::Matrix4 UIElement::GetLocalPositionMatrix() const noexcept {
    const auto origin = GetRenderTransformOrigin();
    const auto transform = GetRenderTransform();
    if (!transform) {
        return NeoWPF::UI::Core::Matrix4::Identity();
    }
    return NeoWPF::UI::Core::Matrix4::Translation(-origin.X, -origin.Y)
        * transform->GetPositionMatrix()
        * NeoWPF::UI::Core::Matrix4::Translation(origin.X, origin.Y);
}

NeoWPF::UI::Core::Matrix4 UIElement::GetWorldTransformMatrix() const noexcept {
    if (m_worldTransformDirty) {
        if (auto parentVisual = GetParent()) {
            if (auto parentElement = dynamic_cast<const UIElement*>(parentVisual)) {
                m_cachedWorldTransform = GetLocalTransformMatrix() * parentElement->GetWorldTransformMatrix();
            } else {
                m_cachedWorldTransform = GetLocalTransformMatrix();
            }
        } else {
            m_cachedWorldTransform = GetLocalTransformMatrix();
        }
        m_worldTransformDirty = false;
    }
    return m_cachedWorldTransform;
}

WorldRenderTransform UIElement::GetWorldRenderTransform() const noexcept {
    WorldRenderTransform result;
    result.positionMatrix = NeoWPF::UI::Core::Matrix4::Identity();
    result.rotationRadians = GetWorldRotationRadians();

    std::vector<const UIElement*> ancestors;
    for (auto current = this; current; current = dynamic_cast<const UIElement*>(current->GetParent())) {
        ancestors.push_back(current);
    }

    bool pivotFound = false;
    for (auto current = ancestors.rbegin(); current != ancestors.rend(); ++current) {
        const auto* element = *current;
        const auto transform = element->GetRenderTransform();
        if (transform && transform->HasRotation() && !pivotFound) {
            const auto origin = element->GetRenderTransformOrigin();
            float originX = origin.X;
            float originY = origin.Y;
            if (const auto* frameworkElement = dynamic_cast<const FrameworkElement*>(element)) {
                originX *= static_cast<float>(frameworkElement->GetFinalWidth());
                originY *= static_cast<float>(frameworkElement->GetFinalHeight());
            }
            element->GetLocalPositionMatrix().TransformPoint(originX, originY, originX, originY);
            result.positionMatrix.TransformPoint(originX, originY, result.pivot.X, result.pivot.Y);
            pivotFound = true;
        }
        result.positionMatrix = element->GetLocalPositionMatrix() * result.positionMatrix;
    }
    return result;
}

float UIElement::GetWorldRotationRadians() const noexcept {
    const auto matrix = GetWorldTransformMatrix();
    std::optional<float> authoredRotation;
    const UIElement* current = this;
    while (current) {
        const auto transform = current->GetRenderTransform();
        if (transform) {
            const auto transformRotation = transform->TryGetRotation();
            if (!transformRotation) {
                return std::atan2(matrix.m[1][0], matrix.m[0][0]);
            }

            if (!authoredRotation) {
                authoredRotation = 0.0f;
            }
            *authoredRotation += *transformRotation;
        }
        current = dynamic_cast<const UIElement*>(current->GetParent());
    }

    if (authoredRotation) {
        return *authoredRotation;
    }

    return std::atan2(matrix.m[1][0], matrix.m[0][0]);
}

namespace {

void ClearTransformInvalidateCallbacks(const std::shared_ptr<NeoWPF::UI::Core::Transform>& transform) {
    if (!transform) {
        return;
    }

    if (auto scaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(transform)) {
        scaleTransform->SetInvalidateCallback(nullptr);
    }
    if (auto rotateTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::RotateTransform>(transform)) {
        rotateTransform->SetInvalidateCallback(nullptr);
    }
    if (auto transformGroup = std::dynamic_pointer_cast<NeoWPF::UI::Core::TransformGroup>(transform)) {
        for (size_t i = 0; i < transformGroup->GetChildCount(); ++i) {
            ClearTransformInvalidateCallbacks(transformGroup->GetChild(i));
        }
    }
}

void SetTransformInvalidateCallbacks(const std::shared_ptr<NeoWPF::UI::Core::Transform>& transform,
                                     const std::function<void()>& callback) {
    if (!transform) {
        return;
    }

    if (auto scaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(transform)) {
        scaleTransform->SetInvalidateCallback(callback);
    }
    if (auto rotateTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::RotateTransform>(transform)) {
        rotateTransform->SetInvalidateCallback(callback);
    }
    if (auto transformGroup = std::dynamic_pointer_cast<NeoWPF::UI::Core::TransformGroup>(transform)) {
        for (size_t i = 0; i < transformGroup->GetChildCount(); ++i) {
            SetTransformInvalidateCallbacks(transformGroup->GetChild(i), callback);
        }
    }
}

}

void UIElement::SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept {
    ClearTransformInvalidateCallbacks(m_renderTransform);

    m_renderTransform = transform;
    SetTransformInvalidateCallbacks(transform, [this]() {
        this->InvalidateVisual();
        this->InvalidateTransformCache();
    });

    InvalidateTransformCache();
    InvalidateVisual();
}

NeoWPF::UI::Core::Point UIElement::GetRenderTransformOrigin() const noexcept {
    return m_renderTransformOrigin;
}

void UIElement::SetRenderTransformOrigin(NeoWPF::UI::Core::Point origin) noexcept {
    m_renderTransformOrigin = origin;
    InvalidateTransformCache();
    InvalidateVisual();
}

void UIElement::SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr) {
    if (m_appliedTemplateRoot) {
        RemoveVisualChild(m_appliedTemplateRoot.get());
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
    if (m_appliedTemplateRoot) {
        AddVisualChild(m_appliedTemplateRoot);
    }
    m_hasTemplateApplied = true;
    return true;
}

void UIElement::ClearTemplateVisualRoot() noexcept {
    if (m_appliedTemplateRoot) {
        RemoveVisualChild(m_appliedTemplateRoot.get());
        m_appliedTemplateRoot = nullptr;
    }
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

std::vector<std::string> UIElement::GetTemplateChildNames() const {
    std::vector<std::string> names;
    names.reserve(m_templateChildren.size());
    for (const auto& entry : m_templateChildren) {
        names.push_back(entry.first);
    }
    return names;
}

void UIElement::ClearTemplateChildren() noexcept {
    m_templateChildren.clear();
    m_hasTemplateApplied = false;
}

bool UIElement::SetStyleProperty(const std::string& name, uint32_t value)
{
    return SetNamedPropertyValue(name, std::any(value), Properties::PropertyValueSource::Style);
}

bool UIElement::SetStylePropertyValue(const std::string& name, const std::any& value)
{
    return SetNamedPropertyValue(name, value, Properties::PropertyValueSource::Style);
}

bool UIElement::SetTriggerPropertyValue(const std::string& name, const std::any& value)
{
    return SetNamedPropertyValue(name, value, Properties::PropertyValueSource::Trigger);
}

bool UIElement::SetNamedPropertyValue(const std::string& name,
                                      const std::any& value,
                                      Properties::PropertyValueSource source)
{
    if (name.empty()) {
        return false;
    }

    auto property = Properties::DependencyPropertyRegistry::Get().FindProperty(name, *this);
    if (!property) {
        return false;
    }

    SetValueInternal(*property, value, source);
    return true;
}

bool UIElement::SetPropertyValue(const std::string& name, const std::any& value)
{
    if (!SetNamedPropertyValue(name, value, Properties::PropertyValueSource::Local)) {
        return false;
    }

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

bool UIElement::SetImplicitXamlStyle(
    const std::type_index& targetType,
    const std::shared_ptr<NeoWPF::UI::Styling::Style>& style)
{
    if (targetType == std::type_index(typeid(void)) || !style) {
        return false;
    }
    m_implicitStyles[targetType] = style;
    return true;
}

bool UIElement::TryFindImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const
{
    const auto it = m_implicitStyles.find(targetType);
    if (it == m_implicitStyles.end()) {
        return false;
    }
    outStyle = it->second;
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

void UIElement::AddEventTrigger(const std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>& trigger)
{
    if (!trigger) {
        return;
    }

    if (std::find(m_eventTriggers.begin(), m_eventTriggers.end(), trigger) == m_eventTriggers.end()) {
        m_eventTriggers.push_back(trigger);
    }
}

void UIElement::RemoveEventTrigger(const std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>& trigger)
{
    if (!trigger) {
        return;
    }

    m_eventTriggers.erase(
        std::remove(m_eventTriggers.begin(), m_eventTriggers.end(), trigger),
        m_eventTriggers.end());
}

std::vector<std::shared_ptr<NeoWPF::UI::Styling::Triggers::EventTrigger>> UIElement::GetEventTriggers() const
{
    return m_eventTriggers;
}

bool UIElement::HasRoutedEventHandler(const NeoWPF::Input::Core::RoutedEvent& routedEvent) const
{
    return m_routedEventHandlers.find(routedEvent.EventTypeId) != m_routedEventHandlers.end();
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

        for (const auto& trigger : element->m_eventTriggers)
        {
            if (!trigger) {
                continue;
            }

            if (trigger->GetRoutedEventName() == args->GetRoutedEvent()->Name) {
                trigger->Invoke(element);
            }
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
