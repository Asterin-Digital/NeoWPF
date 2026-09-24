/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

#if defined(NEOWPF_ENABLE_RUNTIME_XAML)
std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlTree(
	const std::vector<XamlNode>& roots,
	XamlNamescope* namescope = nullptr);
std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlTree(
	const XamlNode& root,
	XamlNamescope* namescope = nullptr);
#endif

}
