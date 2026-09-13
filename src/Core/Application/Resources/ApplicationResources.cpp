#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Styling/Style.h>

namespace NeoWPF::Core::ApplicationResources {

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

void ResourceDictionary::SetImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style> style)
{
    m_implicitStyles[targetType] = std::move(style);
}

bool ResourceDictionary::TryGetImplicitStyle(
    const std::type_index& targetType,
    std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const
{
    const auto it = m_implicitStyles.find(targetType);
    if (it == m_implicitStyles.end()) {
        return false;
    }
    outStyle = it->second;
    return true;
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

bool Resources::TryGetColor(const std::string& key, NeoWPF::UI::Color& outColor)
{
    std::any value;
    if (!TryFindResource(key, value)) {
        return false;
    }

    if (const auto* color = std::any_cast<NeoWPF::UI::Color>(&value)) {
        outColor = *color;
        return true;
    }
    if (const auto* argb = std::any_cast<std::uint32_t>(&value)) {
        outColor = NeoWPF::UI::Color::FromArgb(*argb);
        return true;
    }
    if (const auto* argb = std::any_cast<unsigned int>(&value)) {
        outColor = NeoWPF::UI::Color::FromArgb(*argb);
        return true;
    }
    if (const auto* argb = std::any_cast<int>(&value)) {
        outColor = NeoWPF::UI::Color::FromArgb(static_cast<std::uint32_t>(*argb));
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
