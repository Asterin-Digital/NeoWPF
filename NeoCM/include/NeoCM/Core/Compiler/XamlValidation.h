/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>

#include <string>
#include <vector>

namespace NeoCM::Core::Compiler {

bool ValidateXaml(const std::vector<NeoWPF::UI::Markup::Xaml::XamlNode>& roots,
                 std::string& error);

}
