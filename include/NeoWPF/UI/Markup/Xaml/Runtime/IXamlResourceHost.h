/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <memory>
#include <string>
#include <typeindex>

namespace NeoWPF::UI::Styling {
class Style;
}

namespace NeoWPF::UI::Markup::Xaml {

class IXamlResourceHost {
public:
    virtual ~IXamlResourceHost() = default;

    /*
        @summary
        Stores a keyed resource produced by the XAML loader.

        @param key
        The resource key.

        @param value
        The resource value.

        @returns
        True when the resource was stored.

        @internal
    */
    virtual bool SetXamlResource(const std::string& key, const std::any& value) = 0;

    /*
        @summary
        Looks up a keyed resource produced by the XAML loader.

        @param key
        The resource key.

        @param outValue
        Receives the resource value when found.

        @returns
        True when the resource exists.

        @internal
    */
    virtual bool TryFindXamlResource(const std::string& key, std::any& outValue) const = 0;

    /*
        @summary
        Stores an implicit style keyed by its target type.

        @param targetType
        The type the style targets.

        @param value
        The style value.

        @returns
        True when the style was stored.

        @internal
    */
    virtual bool SetImplicitXamlStyle(
        const std::type_index& targetType,
        const std::shared_ptr<NeoWPF::UI::Styling::Style>& style) = 0;

    /*
        @summary
        Looks up an implicit style keyed by its target type.

        @param targetType
        The type the style targets.

        @param outValue
        Receives the style value when found.

        @returns
        True when a matching style exists.

        @internal
    */
    virtual bool TryFindImplicitStyle(
        const std::type_index& targetType,
        std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const = 0;
};

}