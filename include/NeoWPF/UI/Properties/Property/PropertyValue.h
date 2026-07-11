#pragma once

#include <any>

namespace NeoWPF::UI::Properties {

enum class PropertySource : uint8_t { Default = 0, Style = 1, Local = 2 };

struct PropertyValue {
    std::any Value;
    PropertySource Source = PropertySource::Default;
};

}
