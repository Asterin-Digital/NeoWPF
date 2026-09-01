#pragma once

#include <NeoWPF/UI/Markup/Xaml/XamlNode.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const std::vector<XamlNode>& roots);
std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const XamlNode& root);

}
