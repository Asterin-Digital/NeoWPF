#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemContainerGenerator.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

ItemContainerGenerator::ItemContainerGenerator(NeoWPF::UI::ItemsControl* owner) noexcept
    : m_owner(owner)
{
}

std::shared_ptr<NeoWPF::UI::FrameworkElement> ItemContainerGenerator::Acquire(const std::any& data, int index, const ItemTemplate& templ)
{
    if (!templ) return nullptr;

    if (!m_pool.empty()) {
        auto element = m_pool.back();
        m_pool.pop_back();
        if (element) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
                "[ItemContainerGenerator] Acquire recycled container %p for index %d, re-running template", element.get(), index);
            
    
            auto updatedElement = templ(data, index);
            
            if (updatedElement && updatedElement != element) {
                return updatedElement;
            }

            element->InvalidateMeasure();
            element->InvalidateArrange();
            element->InvalidateVisual();
        }
        return element;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
        "[ItemContainerGenerator] Creating fresh container for index %d", index);
    auto element = templ(data, index);
    return element;
}

void ItemContainerGenerator::Recycle(int index, const std::shared_ptr<NeoWPF::UI::FrameworkElement>& element) noexcept
{
    if (!element) return;
    m_pool.push_back(element);
}

}
