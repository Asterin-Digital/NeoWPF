#pragma once

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlNode.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

#include <cstddef>
#include <string>
#include <vector>

namespace NeoCM::Core::Compiler {

struct ResolvedXamlProperty {
    std::string name;
    std::string value;
    std::string propertyExpression;
    std::string valueExpression;
    std::string bindingPath;
    std::string bindingMode;
    std::string staticResourceKey;
    std::string localeKey;
    bool isMarkupExtension = false;
};

struct ResolvedXamlChild;
struct ResolvedXamlResource;

struct ResolvedXamlElement {
    std::string tagName;
    std::string cppTypeName;
    std::string headerPath;
    std::string xamlName;
    std::vector<ResolvedXamlProperty> properties;
    std::vector<ResolvedXamlChild> children;
    std::vector<ResolvedXamlResource> resources;
    std::string textContent;
    std::size_t line = 0;
    std::size_t column = 0;
};

struct ResolvedXamlChild {
    std::string propertyName;
    std::string propertyExpression;
    std::string collectionAdder;
    ResolvedXamlElement element;
};

struct ResolvedXamlResource {
    std::string key;
    bool isImplicitStyle = false;
    ResolvedXamlElement element;
};

struct ResolvedXamlDocument {
    std::string sourcePath;
    std::string xamlClass;
    NeoWPF::UI::Markup::Xaml::XamlTypeRole rootKind =
        NeoWPF::UI::Markup::Xaml::XamlTypeRole::Element;
    ResolvedXamlElement root;
};

struct XamlResolveResult {
    bool success = false;
    ResolvedXamlDocument document;
    std::string error;
};

XamlResolveResult ResolveXaml(
    const std::vector<NeoWPF::UI::Markup::Xaml::XamlNode>& roots,
    const std::string& sourcePath);

}
