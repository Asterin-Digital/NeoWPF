#pragma once

#include <memory>
#include <functional>
#include <any>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI::Controls::ItemsControl::Virtualization {

using ItemTemplate = std::function<std::shared_ptr<NeoWPF::UI::FrameworkElement>(const std::any& data, int index)>;

}
