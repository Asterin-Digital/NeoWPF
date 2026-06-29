#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI {

void UIElement::InvalidateVisual() {
    m_isDirty = true;
    MarkTreeDirty();
}

void UIElement::ClearDirty() noexcept {
    m_isDirty = false;
    ClearTreeDirty();
}

void UIElement::SetTemplate(const std::shared_ptr<NeoWPF::UI::Templates::ControlTemplateBase>& templatePtr) {
    m_template = templatePtr;
    m_hasTemplateApplied = false;
}

bool UIElement::ApplyTemplate() {
    if (!m_template || m_hasTemplateApplied) {
        return false;
    }

    m_template->Apply(this);
    m_hasTemplateApplied = true;
    return true;
}

void UIElement::RegisterTemplateChild(const std::string& name, Visual* child) {
    if (name.empty() || !child) return;
    m_templateChildren[name] = child;
}

Visual* UIElement::FindTemplateChild(const std::string& name) const {
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

} // namespace NeoWPF::UI
