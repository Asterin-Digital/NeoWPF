#pragma once

#include <any>
#include <string>

namespace NeoWPF::UI::Markup::Xaml {

class IXamlResourceHost {
public:
    virtual ~IXamlResourceHost() = default;

    virtual bool SetXamlResource(const std::string& key, const std::any& value) = 0;
    virtual bool TryFindXamlResource(const std::string& key, std::any& outValue) const = 0;
};

}