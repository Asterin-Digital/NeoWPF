#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>

#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

/* @note
/ Runtime XAML is used by developer's to debug XAML layout/UI issues.
/ NeoCM (Neo Compiler) may produce bad NAML which could lead to issues.
/ NeoPM (Neo Project Manager) may package the XAML in a bad way, which can also lead to issues.
/ With this, we can just pass plain XAML at runtime to see in what layer the issue is in, it is not 
/ meant for public usage and can change in the future.
/* ////////////////////////////////////////////////////
/ It's also used by NeoWPF internally and the compiler, so we need to keep it.
*/

#if defined(NEOWPF_ENABLE_RUNTIME_XAML)
#if !defined(NEOWPF_INTERNAL_BUILD)
#if defined(_MSC_VER)
#pragma message("Warning: Runtime XAML usage is meant for debugging purposes and is not intended to be used by users. Please switch to the actual API when using NeoWPF XAML.")
#elif defined(__GNUC__) || defined(__clang__)
#warning "Runtime XAML usage is meant for debugging purposes and is not intended to be used by users. Please switch to the actual API when using NeoWPF XAML."
#endif
#endif
std::vector<XamlNode> ParseXaml(const std::string& source);
#endif

}
