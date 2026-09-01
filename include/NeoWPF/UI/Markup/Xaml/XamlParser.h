#pragma once

#include <NeoWPF/UI/Markup/Xaml/XamlNode.h>

#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

std::vector<XamlNode> ParseXaml(const std::string& source);

}
