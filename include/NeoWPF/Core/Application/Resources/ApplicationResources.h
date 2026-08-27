#pragma once

#include <any>
#include <string>
#include <unordered_map>

namespace NeoWPF::Core::ApplicationResources {

class ResourceDictionary {
public:
    bool Contains(const std::string& key) const;
    bool TryGet(const std::string& key, std::any& outValue) const;
    void Set(const std::string& key, std::any value);
    std::any Get(const std::string& key) const;
    void Merge(const ResourceDictionary& dictionary);
    void Clear();

private:
    std::unordered_map<std::string, std::any> m_resources;
};

class Resources {
public:
    static void Initialize();
    static void Shutdown();

    static ResourceDictionary& ApplicationResources();
    static bool TryFindResource(const std::string& key, std::any& outValue);
    static std::any FindResource(const std::string& key);
    static void SetResource(const std::string& key, std::any value);
    static bool TryGetResource(const std::string& key, std::any& outValue);

private:
    static ResourceDictionary s_applicationResources;
};

}
