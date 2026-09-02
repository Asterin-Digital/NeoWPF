#include <NeoWPF/UI/Markup/Xaml/XamlMarkupExtension.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>

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

    bool Apply(NeoWPF::UI::Properties::DependencyObject& target,
               const NeoWPF::UI::Properties::DependencyProperty& property,
               std::string& error) const override
    {
        if (NeoWPF::UI::Data::BindingOperations::SetBinding(target, property, m_binding)) {
            return true;
        }
        error = "binding could not be attached to the target property";
        return false;
    }

private:
    NeoWPF::UI::Data::Binding m_binding;
};

std::string Trim(std::string_view value)
{
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return std::string(value.substr(first, last - first));
}

std::vector<std::string> SplitArguments(std::string_view value)
{
    std::vector<std::string> arguments;
    std::size_t start = 0;
    for (std::size_t index = 0; index <= value.size(); ++index) {
        if (index != value.size() && value[index] != ',') {
            continue;
        }

        arguments.push_back(Trim(value.substr(start, index - start)));
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

std::shared_ptr<XamlMarkupExtension> ParseXamlMarkupExtension(
    std::string_view value,
    const XamlMarkupExtensionContext& context,
    std::string& error)
{
    const auto expression = Trim(value);
    if (expression.size() < 3 || expression.front() != '{' || expression.back() != '}') {
        error = "markup extension must be enclosed in '{' and '}'";
        return nullptr;
    }

    const auto body = Trim(std::string_view(expression).substr(1, expression.size() - 2));
    const auto separator = body.find_first_of(" \t\r\n");
    const auto name = body.substr(0, separator);
    const auto* factory = XamlMarkupExtensionRegistry::Get().Find(name);
    if (!factory) {
        error = "markup extension is not registered: " + std::string(name);
        return nullptr;
    }

    return (*factory)(separator == std::string::npos ? std::string_view() : body.substr(separator + 1), context, error);
}

void RegisterXamlMarkupExtensions()
{
    XamlMarkupExtensionRegistry::Get().Register("Binding",
        [](std::string_view arguments,
           const XamlMarkupExtensionContext&,
           std::string& error) {
            std::string path;
            auto mode = NeoWPF::UI::Data::BindingMode::OneWay;
            bool modeSpecified = false;

            for (const auto& argument : SplitArguments(arguments)) {
                if (argument.empty()) {
                    error = "binding contains an empty argument";
                    return std::shared_ptr<XamlMarkupExtension>();
                }

                const auto equals = argument.find('=');
                if (equals == std::string::npos) {
                    if (!path.empty()) {
                        error = "binding path was specified more than once";
                        return std::shared_ptr<XamlMarkupExtension>();
                    }
                    path = Trim(argument);
                    continue;
                }

                const auto key = Trim(std::string_view(argument).substr(0, equals));
                const auto rawValue = Trim(std::string_view(argument).substr(equals + 1));
                if (key == "Path") {
                    if (!path.empty() || rawValue.empty()) {
                        error = "binding Path is missing or duplicated";
                        return std::shared_ptr<XamlMarkupExtension>();
                    }
                    path = rawValue;
                } else if (key == "Mode") {
                    if (modeSpecified || !ParseBindingMode(rawValue, mode)) {
                        error = "binding Mode is invalid or duplicated";
                        return std::shared_ptr<XamlMarkupExtension>();
                    }
                    modeSpecified = true;
                } else {
                    error = "unsupported binding argument: " + key;
                    return std::shared_ptr<XamlMarkupExtension>();
                }
            }

            if (path.empty()) {
                error = "binding Path is required";
                return std::shared_ptr<XamlMarkupExtension>();
            }

            return std::shared_ptr<XamlMarkupExtension>(
                std::make_shared<BindingMarkupExtension>(NeoWPF::UI::Data::Binding(path, mode)));
        });
}

}