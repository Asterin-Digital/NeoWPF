/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>

#include <string>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

/*
    @summary
    Internal runtime XAML parser used for debugging and internal tooling.

    @details
    Runtime XAML is intended for development diagnostics and is not a public API contract.
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
