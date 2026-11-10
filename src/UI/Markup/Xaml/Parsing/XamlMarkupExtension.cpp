#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cctype>
#include <utility>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {
namespace {

class BindingMarkupExtension final : public XamlMarkupExtension {
public:
    explicit BindingMarkupExtension(NeoWPF::UI::Data::Binding binding)
        : m_binding(std::move(binding))
    {
    }

    NeoWPF::Result<void> Apply(
        NeoWPF::UI::Properties::DependencyObject& target,
        const NeoWPF::UI::Properties::DependencyProperty& property) const override
    {
        if (NeoWPF::UI::Data::BindingOperations::SetBinding(target, property, m_binding)) {
            return NeoWPF::Success<void>{};
        }
        return NeoWPF::Fail<std::string>{"binding could not be attached to the target property"};
    }

    const NeoWPF::UI::Data::Binding& GetBinding() const noexcept
    {
        return m_binding;
    }

private:
    NeoWPF::UI::Data::Binding m_binding;
};

class StaticResourceMarkupExtension final : public XamlMarkupExtension {
public:
    StaticResourceMarkupExtension(std::string key, XamlResourceResolver resolver)
        : m_key(std::move(key))
        , m_resolver(std::move(resolver))
    {
    }

    NeoWPF::Result<void> Apply(
        NeoWPF::UI::Properties::DependencyObject& target,
        const NeoWPF::UI::Properties::DependencyProperty& property) const override
    {
        if (!m_resolver) {
            return NeoWPF::Fail<std::string>{"StaticResource has no resource scope"};
        }

        std::any value;
        if (!m_resolver(m_key, value)) {
            return NeoWPF::Fail<std::string>{"resource was not found: " + m_key};
        }

        if (const auto* binding = property.GetXamlBinding()) {
            if (const auto* resourceObject =
                    std::any_cast<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>(&value)) {
                binding->Assign(target, *resourceObject);
                return NeoWPF::Success<void>{};
            }
        }

        target.SetValue(property, value);
        return NeoWPF::Success<void>{};
    }

    const std::string& GetKey() const noexcept
    {
        return m_key;
    }

private:
    std::string m_key;
    XamlResourceResolver m_resolver;
};

class LocaleMarkupExtension final : public XamlMarkupExtension {
public:
    explicit LocaleMarkupExtension(std::wstring key)
        : m_key(std::move(key))
    {
    }

    NeoWPF::Result<void> Apply(
        NeoWPF::UI::Properties::DependencyObject& target,
        const NeoWPF::UI::Properties::DependencyProperty& property) const override
    {
        if (!NeoWPF::UI::Localization::LocaleOperations::SetLocale(target, property, m_key)) {
            return NeoWPF::Fail<std::string>{"Locale could not be attached to the target property"};
        }
        return NeoWPF::Success<void>{};
    }

    const std::wstring& GetKey() const noexcept
    {
        return m_key;
    }

private:
    std::wstring m_key;
};

std::vector<std::string> SplitArguments(std::string_view value)
{
    std::vector<std::string> arguments;
    std::size_t start = 0;
    for (std::size_t index = 0; index <= value.size(); ++index) {
        if (index != value.size() && value[index] != ',') {
            continue;
        }

        arguments.push_back(NeoWPF::Platform::Trim(
            std::string(value.substr(start, index - start))));
        start = index + 1;
    }
    return arguments;
}

bool ParseBindingMode(std::string_view value, NeoWPF::UI::Data::BindingMode& mode)
{
    std::string normalized(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });

    if (normalized == "oneway") {
        mode = NeoWPF::UI::Data::BindingMode::OneWay;
        return true;
    }
    if (normalized == "twoway") {
        mode = NeoWPF::UI::Data::BindingMode::TwoWay;
        return true;
    }
    if (normalized == "onetime") {
        mode = NeoWPF::UI::Data::BindingMode::OneTime;
        return true;
    }
    return false;
}

}

XamlMarkupExtensionRegistry& XamlMarkupExtensionRegistry::Get()
{
    static XamlMarkupExtensionRegistry instance;
    return instance;
}

void XamlMarkupExtensionRegistry::Register(std::string_view name, XamlMarkupExtensionFactory factory)
{
    if (name.empty() || !factory || m_factories.find(std::string(name)) != m_factories.end()) {
        return;
    }
    m_factories.emplace(std::string(name), std::move(factory));
}

const XamlMarkupExtensionFactory* XamlMarkupExtensionRegistry::Find(std::string_view name) const
{
    const auto iterator = m_factories.find(std::string(name));
    return iterator == m_factories.end() ? nullptr : &iterator->second;
}

NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>> ParseXamlMarkupExtension(
    std::string_view value,
    const XamlMarkupExtensionContext& context)
{
    const auto expression = NeoWPF::Platform::Trim(std::string(value));
    if (expression.size() < 3 || expression.front() != '{' || expression.back() != '}') {
        return NeoWPF::Fail<std::string>{"markup extension must be enclosed in '{' and '}'"};
    }

    const auto body = NeoWPF::Platform::Trim(
        std::string(std::string_view(expression).substr(1, expression.size() - 2)));
    const auto separator = body.find_first_of(" \t\r\n");
    const auto name = body.substr(0, separator);
    const auto* factory = XamlMarkupExtensionRegistry::Get().Find(name);
    if (!factory) {
        return NeoWPF::Fail<std::string>{"markup extension is not registered: " + std::string(name)};
    }

    return (*factory)(separator == std::string::npos ? std::string_view() : body.substr(separator + 1), context);
}

bool TryGetXamlBinding(const XamlMarkupExtension& extension, XamlBindingInfo& outBinding)
{
    const auto* binding = dynamic_cast<const BindingMarkupExtension*>(&extension);
    if (!binding) {
        return false;
    }

    outBinding.Path = binding->GetBinding().GetPath();
    switch (binding->GetBinding().GetMode()) {
    case NeoWPF::UI::Data::BindingMode::OneWay: outBinding.Mode = "OneWay"; break;
    case NeoWPF::UI::Data::BindingMode::TwoWay: outBinding.Mode = "TwoWay"; break;
    case NeoWPF::UI::Data::BindingMode::OneTime: outBinding.Mode = "OneTime"; break;
    }
    return true;
}

bool TryGetXamlStaticResource(const XamlMarkupExtension& extension, std::string& outKey)
{
    const auto* resource = dynamic_cast<const StaticResourceMarkupExtension*>(&extension);
    if (!resource) {
        return false;
    }

    outKey = resource->GetKey();
    return true;
}

bool TryGetXamlLocaleKey(const XamlMarkupExtension& extension, std::string& outKey)
{
    const auto* locale = dynamic_cast<const LocaleMarkupExtension*>(&extension);
    if (!locale) {
        return false;
    }

    outKey = NeoWPF::Platform::WideToUtf8(locale->GetKey());
    return !outKey.empty();
}

void RegisterXamlMarkupExtensions()
{
    XamlMarkupExtensionRegistry::Get().Register("Binding",
        [](std::string_view arguments,
           const XamlMarkupExtensionContext&) -> NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>> {
            std::string path;
            auto mode = NeoWPF::UI::Data::BindingMode::OneWay;
            bool modeSpecified = false;

            for (const auto& argument : SplitArguments(arguments)) {
                if (argument.empty()) {
                    return NeoWPF::Fail<std::string>{"binding contains an empty argument"};
                }

                const auto equals = argument.find('=');
                if (equals == std::string::npos) {
                    if (!path.empty()) {
                        return NeoWPF::Fail<std::string>{"binding path was specified more than once"};
                    }
                    path = NeoWPF::Platform::Trim(argument);
                    continue;
                }

                const auto key = NeoWPF::Platform::Trim(
                    std::string(std::string_view(argument).substr(0, equals)));
                const auto rawValue = NeoWPF::Platform::Trim(
                    std::string(std::string_view(argument).substr(equals + 1)));
                if (key == "Path") {
                    if (!path.empty() || rawValue.empty()) {
                        return NeoWPF::Fail<std::string>{"binding Path is missing or duplicated"};
                    }
                    path = rawValue;
                } else if (key == "Mode") {
                    if (modeSpecified || !ParseBindingMode(rawValue, mode)) {
                        return NeoWPF::Fail<std::string>{"binding Mode is invalid or duplicated"};
                    }
                    modeSpecified = true;
                } else {
                    return NeoWPF::Fail<std::string>{"unsupported binding argument: " + key};
                }
            }

            if (path.empty()) {
                return NeoWPF::Fail<std::string>{"binding Path is required"};
            }

            return NeoWPF::Success<std::shared_ptr<XamlMarkupExtension>>{
                std::make_shared<BindingMarkupExtension>(NeoWPF::UI::Data::Binding(path, mode))};
        });

    XamlMarkupExtensionRegistry::Get().Register("StaticResource",
        [](std::string_view arguments,
           const XamlMarkupExtensionContext& context) -> NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>> {
            const auto key = NeoWPF::Platform::Trim(std::string(arguments));
            if (key.empty()) {
                return NeoWPF::Fail<std::string>{"StaticResource key is required"};
            }
            return NeoWPF::Success<std::shared_ptr<XamlMarkupExtension>>{
                std::make_shared<StaticResourceMarkupExtension>(key, context.ResolveResource)};
        });

    XamlMarkupExtensionRegistry::Get().Register("Locale",
        [](std::string_view arguments,
           const XamlMarkupExtensionContext&) -> NeoWPF::Result<std::shared_ptr<XamlMarkupExtension>> {
            const auto trimmedArguments = NeoWPF::Platform::Trim(std::string(arguments));
            if (trimmedArguments.empty()) {
                return NeoWPF::Fail<std::string>{"Locale key is required"};
            }

            std::string key;
            for (const auto& argument : SplitArguments(trimmedArguments)) {
                if (argument.empty()) {
                    return NeoWPF::Fail<std::string>{"Locale contains an empty argument"};
                }

                const auto equals = argument.find('=');
                if (equals == std::string::npos) {
                    if (!key.empty()) {
                        return NeoWPF::Fail<std::string>{"Locale key was specified more than once"};
                    }
                    key = NeoWPF::Platform::Trim(argument);
                    continue;
                }

                const auto name = NeoWPF::Platform::Trim(
                    std::string(std::string_view(argument).substr(0, equals)));
                const auto value = NeoWPF::Platform::Trim(
                    std::string(std::string_view(argument).substr(equals + 1)));
                if (name != "Key" || value.empty() || !key.empty()) {
                    return NeoWPF::Fail<std::string>{"Locale Key is invalid or duplicated"};
                }
                key = value;
            }

            if (key.empty()) {
                return NeoWPF::Fail<std::string>{"Locale key is required"};
            }

            const auto wideKey = NeoWPF::Platform::Utf8ToWide(key);
            if (wideKey.empty()) {
                return NeoWPF::Fail<std::string>{"Locale key is not valid UTF-8"};
            }
            return NeoWPF::Success<std::shared_ptr<XamlMarkupExtension>>{
                std::make_shared<LocaleMarkupExtension>(wideKey)};
        });
}

}