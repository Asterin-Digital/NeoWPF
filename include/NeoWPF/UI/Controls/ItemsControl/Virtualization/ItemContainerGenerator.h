/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/ItemsControl/Virtualization/ItemTemplate.h>

namespace NeoWPF::UI { class ItemsControl; }

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

class ItemContainerGenerator {
public:
    explicit ItemContainerGenerator(NeoWPF::UI::ItemsControl* owner) noexcept;

    std::shared_ptr<NeoWPF::UI::FrameworkElement> Acquire(const std::any& data, int index, const ItemTemplate& templ);
    void Recycle(int index, const std::shared_ptr<NeoWPF::UI::FrameworkElement>& element) noexcept;

private:
    NeoWPF::UI::ItemsControl* m_owner;
    std::unordered_map<int, std::weak_ptr<NeoWPF::UI::FrameworkElement>> m_activeContainers;
};

}
