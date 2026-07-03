#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

namespace NeoWPF::UI::Templates {

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

ControlTemplate::ControlTemplate() = default;

bool ControlTemplate::CanApplyTo(UIElement* element) const
{
    return ControlTemplateBase::CanApplyTo(element);
}

Visual* ControlTemplate::GetVisualTreeRoot() const noexcept
{
    return m_visualTree.get();
}

void ControlTemplate::Apply(UIElement* element)
{
    if (!element) {
        return;
    }

    element->ClearTemplateChildren();

    if (!m_visualTree) {
        m_visualTree = std::make_shared<Visual>();
    }

    for (const auto& child : m_templateChildren) {
        if (child.Name.empty() || !child.Visual || child.Visual.get() == m_visualTree.get()) {
            continue;
        }

        m_visualTree->AddVisualChild(child.Visual.get());
        element->RegisterTemplateChild(child.Name, child.Visual.get());
    }
}

void ControlTemplate::SetVisualTree(std::shared_ptr<Visual> visualTree)
{
    m_visualTree = std::move(visualTree);
}

std::shared_ptr<Visual> ControlTemplate::GetVisualTree() const noexcept
{
    return m_visualTree;
}

void ControlTemplate::AddVisualChild(std::shared_ptr<Visual> visualChild)
{
    if (!visualChild) {
        return;
    }

    if (!m_visualTree) {
        m_visualTree = std::make_shared<Visual>();
    }

    m_visualTree->AddVisualChild(visualChild.get());
}

void ControlTemplate::AddTemplateChild(const std::string& name, std::shared_ptr<Visual> visualChild)
{
    if (name.empty() || !visualChild) {
        return;
    }

    m_templateChildren.push_back({name, std::move(visualChild)});
}

} // namespace NeoWPF::UI::Templates
