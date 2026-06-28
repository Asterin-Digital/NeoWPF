#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/UIElement.h>
#include <NeoWPF/UI/Core/Visual.h>
#include <utility>

namespace NeoWPF::UI::Controls::Templates {

bool ControlTemplateBase::CanApplyTo(UIElement* element) const
{
    return element != nullptr;
}

void ControlTemplateBase::AddRequiredPart(const std::string& name, bool required)
{
    if (name.empty()) return;
    m_requiredParts.push_back({name, required});
}

bool ControlTemplateBase::HasRequiredParts() const noexcept
{
    return !m_requiredParts.empty();
}

const std::vector<TemplatePart>& ControlTemplateBase::GetRequiredParts() const noexcept
{
    return m_requiredParts;
}

ControlTemplateBuilder::ControlTemplateBuilder(UIElement* control)
    : m_control(control)
{
}

UIElement* ControlTemplateBuilder::GetControl() const noexcept
{
    return m_control;
}

void ControlTemplateBuilder::RegisterPart(const std::string& name, Visual* child)
{
    if (!m_control || name.empty() || !child) return;
    m_control->RegisterTemplateChild(name, child);
}

void ControlTemplateBuilder::AttachChild(Visual* child, const std::string& partName)
{
    if (!m_control || !child) return;
    m_control->AddVisualChild(child);
    if (!partName.empty()) {
        RegisterPart(partName, child);
    }
}

ControlTemplate::ControlTemplate(Builder builder)
    : m_builder(std::move(builder))
{
}

bool ControlTemplate::CanApplyTo(UIElement* element) const
{
    return ControlTemplateBase::CanApplyTo(element);
}

void ControlTemplate::Apply(UIElement* element)
{
    if (!element || !m_builder) {
        return;
    }

    ControlTemplateBuilder builder(element);
    m_builder(element, builder);
}

} // namespace NeoWPF::UI::Controls::Templates
