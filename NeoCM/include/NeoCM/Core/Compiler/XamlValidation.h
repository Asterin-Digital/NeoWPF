#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>

#include <string>
#include <vector>

namespace NeoCM::Core::Compiler {

bool ValidateXaml(const std::vector<NeoWPF::UI::Markup::Xaml::XamlNode>& roots,
                 std::string& error);

}
