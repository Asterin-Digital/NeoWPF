#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlNamescope.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

struct XamlBuildContext {
    std::vector<NeoWPF::UI::UIElement*> scopes;
};

using XamlObjectBuilder = std::function<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>(
    const XamlNode&,
    XamlNamescope*,
    XamlBuildContext&)>;

bool IsXamlResourcesProperty(std::string_view propertyName);

XamlResourceResolver CreateXamlResourceResolver(
    NeoWPF::UI::Properties::DependencyObject& target,
    const XamlBuildContext& context);

bool BuildXamlResourceDictionary(
    const XamlNode& propertyNode,
    NeoWPF::UI::Properties::DependencyObject& owner,
    XamlNamescope* namescope,
    XamlBuildContext& context,
    const XamlObjectBuilder& buildObject);

}
