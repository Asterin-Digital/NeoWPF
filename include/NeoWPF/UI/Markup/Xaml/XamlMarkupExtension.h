#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace NeoWPF::UI::Properties {
class DependencyObject;
class DependencyProperty;
}

namespace NeoWPF::UI::Markup::Xaml {

struct XamlMarkupExtensionContext {
    std::size_t Line = 0;
    std::size_t Column = 0;
};

class XamlMarkupExtension {
public:
    virtual ~XamlMarkupExtension() = default;

    virtual bool Apply(NeoWPF::UI::Properties::DependencyObject& target,
                       const NeoWPF::UI::Properties::DependencyProperty& property,
                       std::string& error) const = 0;
};

using XamlMarkupExtensionFactory = std::function<std::shared_ptr<XamlMarkupExtension>(
    std::string_view,
    const XamlMarkupExtensionContext&,
    std::string&)>;

class XamlMarkupExtensionRegistry {
public:
    static XamlMarkupExtensionRegistry& Get();

    void Register(std::string_view name, XamlMarkupExtensionFactory factory);
    const XamlMarkupExtensionFactory* Find(std::string_view name) const;

private:
    XamlMarkupExtensionRegistry() = default;

    std::unordered_map<std::string, XamlMarkupExtensionFactory> m_factories;
};

std::shared_ptr<XamlMarkupExtension> ParseXamlMarkupExtension(
    std::string_view value,
    const XamlMarkupExtensionContext& context,
    std::string& error);

void RegisterXamlMarkupExtensions();

}