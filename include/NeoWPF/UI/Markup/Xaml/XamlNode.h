#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

struct XamlNode {
    std::string Name;
    std::unordered_map<std::string, std::string> Attributes;
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
