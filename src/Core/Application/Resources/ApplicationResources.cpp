#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Styling/Style.h>

namespace NeoWPF::Core::ApplicationResources {

ResourceDictionary Resources::s_applicationResources;

bool ResourceDictionary::Contains(const std::string& key) const
{
    std::shared_lock lock(m_mutex);
    return m_resources.find(key) != m_resources.end();
}

bool ResourceDictionary::TryGet(const std::string& key, std::any& outValue) const
{
    std::shared_lock lock(m_mutex);
    const auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        outValue = it->second;
        return true;
    }
    return false;
}

void ResourceDictionary::Set(const std::string& key, std::any value)
{
    std::unique_lock lock(m_mutex);
    m_resources[key] = std::move(value);
}

void ResourceDictionary::SetMany(const std::unordered_map<std::string, std::any>& values)
{
    std::unique_lock lock(m_mutex);
    for (const auto& [key, value] : values) {
        m_resources[key] = value;
    }
}

void ResourceDictionary::SetImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style> style)
{
    std::unique_lock lock(m_mutex);
    m_implicitStyles[targetType] = std::move(style);
}

bool ResourceDictionary::TryGetImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const
{
    std::shared_lock lock(m_mutex);
    const auto it = m_implicitStyles.find(targetType);
    if (it == m_implicitStyles.end()) {
        return false;
    }
    outStyle = it->second;
    return true;
}

std::any ResourceDictionary::Get(const std::string& key) const
{
    std::shared_lock lock(m_mutex);
    const auto it = m_resources.find(key);
    if (it != m_resources.end()) {
        return it->second;
    }
    return {};
}

void ResourceDictionary::Merge(const ResourceDictionary& dictionary)
{
    std::unique_lock lock(m_mutex);
    for (const auto& [key, value] : dictionary.m_resources) {
        m_resources[key] = value;
    }
}

void ResourceDictionary::Clear()
{
    std::unique_lock lock(m_mutex);
    m_resources.clear();
    m_implicitStyles.clear();
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

bool Resources::TryGetBrush(
    const std::string& key,
    std::shared_ptr<NeoWPF::UI::Brush>& outBrush)
{
    std::any value;
    if (!TryFindResource(key, value)) {
        return false;
    }

    if (const auto* brush = std::any_cast<std::shared_ptr<NeoWPF::UI::Brush>>(&value)) {
        outBrush = *brush;
        return true;
    }

    return false;
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

void Resources::SetResources(const std::unordered_map<std::string, std::any>& values)
{
    s_applicationResources.SetMany(values);
}

void Resources::SetImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style> style)
{
    s_applicationResources.SetImplicitStyle(targetType, std::move(style));
}

bool Resources::TryFindImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle)
{
    return s_applicationResources.TryGetImplicitStyle(targetType, outStyle);
}

bool Resources::TryGetResource(const std::string& key, std::any& outValue)
{
    return s_applicationResources.TryGet(key, outValue);
}

}
