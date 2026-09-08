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
    if (!templ) {
        return nullptr;
    }

    auto container = templ(data, index);
    if (!container) {
        return nullptr;
    }

    container->SetDataContext(data);
    m_activeContainers[index] = container;
    return container;
}

void ItemContainerGenerator::Recycle(int index, const std::shared_ptr<NeoWPF::UI::FrameworkElement>& element) noexcept
{
    if (!element) {
        m_activeContainers.erase(index);
        return;
    }

    element->SetDataContext(std::any{});
    element->SetLayoutOffset(0, 0);

    auto active = m_activeContainers.find(index);
    if (active != m_activeContainers.end()) {
        auto activeContainer = active->second.lock();
        if (!activeContainer || activeContainer.get() == element.get()) {
            m_activeContainers.erase(active);
        }
    }
}

}
