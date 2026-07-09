#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/Property/PropertyRegistry.h>
#include <NeoWPF/UI/Properties/Property/PropertyInfo.h>

namespace NeoWPF::UI {

void UIElement::InvalidateVisual() {
    m_isDirty = true;
    MarkTreeDirty();
}

void UIElement::ClearDirty() noexcept {
    m_isDirty = false;
    ClearTreeDirty();
}

void UIElement::BeginAnimation(float from, float to, float durationSeconds) {
    using namespace NeoWPF::UI::Animation;
    auto storyboard = std::make_shared<Storyboard>();
    auto timeline = std::make_shared<Timeline>(from, to, durationSeconds, [this](float value) {
        this->SetOpacity(value);
    });
    storyboard->AddTimeline(timeline);
    NeoWPF::UI::AddStoryboard(storyboard);
}

std::shared_ptr<NeoWPF::UI::Core::Transform> UIElement::GetRenderTransform() const noexcept {
    return m_renderTransform;
}

void UIElement::SetRenderTransform(std::shared_ptr<NeoWPF::UI::Core::Transform> transform) noexcept {
    if (auto oldScaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(m_renderTransform)) {
        oldScaleTransform->m_invalidateCallback = nullptr;
    }

    m_renderTransform = transform;
    if (auto scaleTransform = std::dynamic_pointer_cast<NeoWPF::UI::Core::ScaleTransform>(transform)) {
        scaleTransform->m_invalidateCallback = [this]() {
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
        RemoveVisualChild(m_appliedTemplateRoot);
        m_appliedTemplateRoot = nullptr;
    }

    m_template = templatePtr;
    m_hasTemplateApplied = false;
}

bool UIElement::ApplyTemplate() {
    if (!m_template || m_hasTemplateApplied) {
        return false;
    }

    if (m_appliedTemplateRoot) {
        RemoveVisualChild(m_appliedTemplateRoot);
        m_appliedTemplateRoot = nullptr;
    }

    m_template->Apply(this);
    m_appliedTemplateRoot = m_template->GetVisualTreeRoot(); m_hasTemplateApplied = true;
    return true;
}

void UIElement::ClearTemplateVisualRoot() noexcept {
    if (m_appliedTemplateRoot) {
        RemoveVisualChild(m_appliedTemplateRoot);
        m_appliedTemplateRoot = nullptr;
    }
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

void UIElement::SetProperty(const std::string& name, uint32_t value) {
    if (name.empty()) {
        return;
    }
    m_properties[name] = value;
}

bool UIElement::TryGetProperty(const std::string& name, uint32_t& outValue) const {
    if (name.empty()) {
        return false;
    }
    auto it = m_properties.find(name);
    if (it == m_properties.end()) {
        return false;
    }
    outValue = it->second;
    return true;
}

bool UIElement::SetPropertyValue(const std::string& name, const std::any& value) {
    auto& registry = Properties::PropertyRegistry::Get(typeid(*this));
    if (!registry.SetPropertyValue(this, name, value)) {
        return false;
    }
    PropertyChanged.Invoke(name, value);
    return true;
}

std::any UIElement::GetPropertyValue(const std::string& name) const {
    auto& registry = Properties::PropertyRegistry::Get(typeid(*this));
    return registry.GetPropertyValue(const_cast<UIElement*>(this), name);
}

std::vector<Properties::PropertyInfo> UIElement::GetPropertyInfo() const {
    auto& registry = Properties::PropertyRegistry::Get(typeid(*this));
    return registry.GetProperties();
}

}
