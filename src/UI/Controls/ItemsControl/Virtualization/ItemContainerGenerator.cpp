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

    return templ(data, index);
}

void ItemContainerGenerator::Recycle(int index, const std::shared_ptr<NeoWPF::UI::FrameworkElement>& element) noexcept
{
    (void)index;
    (void)element;
}

}
