#include <NeoWPF/Core/Application/ApplicationResources.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::Core::Application {

ResourceDictionary Resources::s_applicationResources;

bool ResourceDictionary::Contains(const std::string& key) const
{
    return m_resources.find(key) != m_resources.end();
}

bool ResourceDictionary::TryGet(const std::string& key, std::any& outValue) const
{
    const auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

void ResourceDictionary::Set(const std::string& key, std::any value)
{
    m_resources[key] = std::move(value);
}

std::any ResourceDictionary::Get(const std::string& key) const
{
    const auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        return it->second;
    }
    return {};
}

void ResourceDictionary::Merge(const ResourceDictionary& dictionary)
{
    for (const auto& [key, value] : dictionary.m_resources) {
        m_resources[key] = value;
    }
}

void ResourceDictionary::Clear()
{
    m_resources.clear();
}

void Resources::Initialize()
{
    NeoWPF::Diagnostics::LogInfo("Resources::Initialize");
    s_applicationResources.Clear();
}

void Resources::Shutdown()
{
    NeoWPF::Diagnostics::LogInfo("Resources::Shutdown");
    s_applicationResources.Clear();
}

ResourceDictionary& Resources::ApplicationResources()
{
    return s_applicationResources;
}

bool Resources::TryFindResource(const std::string& key, std::any& outValue)
{
    return s_applicationResources.TryGet(key, outValue);
}

std::any Resources::FindResource(const std::string& key)
{
    std::any value;
    if (TryFindResource(key, value)) {
        return value;
    }
    return {};
}

void Resources::SetResource(const std::string& key, std::any value)
{
    s_applicationResources.Set(key, std::move(value));
}

bool Resources::TryGetResource(const std::string& key, std::any& outValue)
{
    return s_applicationResources.TryGet(key, outValue);
}

}
