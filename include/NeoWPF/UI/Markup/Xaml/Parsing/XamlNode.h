/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

struct XamlNode {
    std::string Name;
    std::string XamlName;
    std::string XamlKey;
    std::unordered_map<std::string, std::string> Attributes;
    std::string XamlClass;
    std::vector<XamlNode> Children;
    std::string TextContent;
    std::size_t Line = 0;
    std::size_t Column = 0;

    bool IsTextNode() const noexcept
    {
        return Name.empty() && !TextContent.empty();
    }
};

}
