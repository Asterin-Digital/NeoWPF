/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <memory>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <string>
#include <unordered_map>
#include <typeindex>

namespace NeoWPF::UI::Styling {
class Style;
}

namespace NeoWPF::Core::ApplicationResources {

class ResourceDictionary {
public:
    bool Contains(const std::string& key) const;
    bool TryGet(const std::string& key, std::any& outValue) const;
    void Set(const std::string& key, std::any value);
    void SetImplicitStyle(const std::type_index& targetType,
                          std::shared_ptr<NeoWPF::UI::Styling::Style> style);
    bool TryGetImplicitStyle(
        const std::type_index& targetType,
        std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const;
    std::any Get(const std::string& key) const;
    void Merge(const ResourceDictionary& dictionary);
    void Clear();

private:
    std::unordered_map<std::string, std::any> m_resources;
    std::unordered_map<std::type_index, std::shared_ptr<NeoWPF::UI::Styling::Style>> m_implicitStyles;
};

class Resources {
public:
    static void Initialize();
    static void Shutdown();

    static ResourceDictionary& ApplicationResources();
    static bool TryFindResource(const std::string& key, std::any& outValue);
    static bool TryGetColor(const std::string& key, NeoWPF::UI::Color& outColor);
    static std::any FindResource(const std::string& key);
    static void SetResource(const std::string& key, std::any value);
    static void SetImplicitStyle(
        const std::type_index& targetType,
        std::shared_ptr<NeoWPF::UI::Styling::Style> style);
    static bool TryFindImplicitStyle(
        const std::type_index& targetType,
        std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle);
    static bool TryGetResource(const std::string& key, std::any& outValue);

private:
    static ResourceDictionary s_applicationResources;
};

}
