#pragma once

#include <string>
#include <string_view>

namespace NeoWPF::UI::Markup::Xaml {

std::string DecodeXmlEntities(std::string_view value);

}
