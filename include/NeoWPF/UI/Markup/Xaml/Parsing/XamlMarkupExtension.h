/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <NeoWPF/Etc/Result.h>

namespace NeoWPF::UI::Properties {
class DependencyObject;
class DependencyProperty;
}

namespace NeoWPF::UI::Markup::Xaml {

using XamlResourceResolver = std::function<bool(std::string_view, std::any&)>;

struct XamlMarkupExtensionContext {
    std::size_t Line = 0;
    std::size_t Column = 0;
    XamlResourceResolver ResolveResource;
};

struct XamlBindingInfo {
    std::string Path;
    std::string Mode;
};

class XamlMarkupExtension {
public:
    virtual ~XamlMarkupExtension() = default;

    /*
        @summary
        Applies this markup extension to a dependency property.

        @param target
        Object receiving the extension value.

        @param property
        Property receiving the extension value.

        @returns
        Success when the value is applied, or the failure reason.
    */
    virtual NeoWPF::Result<void> Apply(
        NeoWPF::UI::Properties::DependencyObject& target,
        const NeoWPF::UI::Properties::DependencyProperty& property) const = 0;
};

/*
    @summary
    Creates a markup extension from its arguments and parsing context.

    @param arguments
    Text following the extension name.

    @param context
    Context available while creating the extension.

    @returns
    The created extension, or the failure reason.
*/
using XamlMarkupExtensionFactory = std::function<NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>>(
    std::string_view,
    const XamlMarkupExtensionContext&)>;

class XamlMarkupExtensionRegistry {
public:
    static XamlMarkupExtensionRegistry& Get();

    void Register(std::string_view name, XamlMarkupExtensionFactory factory);
    const XamlMarkupExtensionFactory* Find(std::string_view name) const;

private:
    XamlMarkupExtensionRegistry() = default;

    std::unordered_map<std::string, XamlMarkupExtensionFactory> m_factories;
};

/*
    @summary
    Parses a XAML markup extension expression.

    @param value
    Markup extension expression.

    @param context
    Context available while parsing the extension.

    @returns
    The parsed extension, or the failure reason.
*/
NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>> ParseXamlMarkupExtension(
    std::string_view value,
    const XamlMarkupExtensionContext& context);

bool TryGetXamlBinding(const XamlMarkupExtension& extension, XamlBindingInfo& outBinding);

bool TryGetXamlStaticResource(const XamlMarkupExtension& extension, std::string& outKey);

bool TryGetXamlLocaleKey(const XamlMarkupExtension& extension, std::string& outKey);

void RegisterXamlMarkupExtensions();

}