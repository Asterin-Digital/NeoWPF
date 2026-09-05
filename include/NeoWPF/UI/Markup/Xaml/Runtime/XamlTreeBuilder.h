#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

#if defined(NEOWPF_ENABLE_RUNTIME_XAML)
std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const std::vector<XamlNode>& roots,
												 XamlNamescope* namescope = nullptr);
std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const XamlNode& root,
												 XamlNamescope* namescope = nullptr);
#endif

}
