#include <NeoWPF/UI/Layout/Responsive/ResponsiveRule.h>
#include <NeoWPF/UI/Properties/TypeConverter.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <memory>

namespace {

bool ParseScope(const std::string& rawValue, NeoWPF::UI::ResponsiveScope& scope)
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "container") {
        scope = NeoWPF::UI::ResponsiveScope::Container;
        return true;
    }
    if (value == "window") {
        scope = NeoWPF::UI::ResponsiveScope::Window;
        return true;
    }
    return false;
}

bool ParsePointer(const std::string& rawValue, NeoWPF::UI::ResponsivePointer& pointer)
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "any") {
        pointer = NeoWPF::UI::ResponsivePointer::Any;
        return true;
    }
    if (value == "fine") {
        pointer = NeoWPF::UI::ResponsivePointer::Fine;
        return true;
    }
    if (value == "coarse") {
        pointer = NeoWPF::UI::ResponsivePointer::Coarse;
        return true;
    }
    return false;
}

bool ParseHover(const std::string& rawValue, NeoWPF::UI::ResponsiveHover& hover)
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "any") {
        hover = NeoWPF::UI::ResponsiveHover::Any;
        return true;
    }
    if (value == "on") {
        hover = NeoWPF::UI::ResponsiveHover::On;
        return true;
    }
    if (value == "off") {
        hover = NeoWPF::UI::ResponsiveHover::Off;
        return true;
    }
    return false;
}

bool ParseKeyboard(const std::string& rawValue, NeoWPF::UI::ResponsiveKeyboard& keyboard)
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "any") {
        keyboard = NeoWPF::UI::ResponsiveKeyboard::Any;
        return true;
    }
    if (value == "shown") {
        keyboard = NeoWPF::UI::ResponsiveKeyboard::Shown;
        return true;
    }
    if (value == "hidden") {
        keyboard = NeoWPF::UI::ResponsiveKeyboard::Hidden;
        return true;
    }
    return false;
}

class ScopeConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::ResponsiveScope scope{};
        if (!ParseScope(rawValue, scope)) {
            return false;
        }
        convertedValue = scope;
        return true;
    }

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override
    {
        NeoWPF::UI::ResponsiveScope scope{};
        if (!ParseScope(rawValue, scope)) {
            return false;
        }
        expression = scope == NeoWPF::UI::ResponsiveScope::Container
            ? "NeoWPF::UI::ResponsiveScope::Container"
            : "NeoWPF::UI::ResponsiveScope::Window";
        return true;
    }
};

class PointerConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::ResponsivePointer pointer{};
        if (!ParsePointer(rawValue, pointer)) {
            return false;
        }
        convertedValue = pointer;
        return true;
    }

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override
    {
        NeoWPF::UI::ResponsivePointer pointer{};
        if (!ParsePointer(rawValue, pointer)) {
            return false;
        }
        switch (pointer) {
        case NeoWPF::UI::ResponsivePointer::Any:
            expression = "NeoWPF::UI::ResponsivePointer::Any";
            return true;
        case NeoWPF::UI::ResponsivePointer::Fine:
            expression = "NeoWPF::UI::ResponsivePointer::Fine";
            return true;
        case NeoWPF::UI::ResponsivePointer::Coarse:
            expression = "NeoWPF::UI::ResponsivePointer::Coarse";
            return true;
        }
        return false;
    }
};

class HoverConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::ResponsiveHover hover{};
        if (!ParseHover(rawValue, hover)) {
            return false;
        }
        convertedValue = hover;
        return true;
    }

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override
    {
        NeoWPF::UI::ResponsiveHover hover{};
        if (!ParseHover(rawValue, hover)) {
            return false;
        }
        switch (hover) {
        case NeoWPF::UI::ResponsiveHover::Any:
            expression = "NeoWPF::UI::ResponsiveHover::Any";
            return true;
        case NeoWPF::UI::ResponsiveHover::On:
            expression = "NeoWPF::UI::ResponsiveHover::On";
            return true;
        case NeoWPF::UI::ResponsiveHover::Off:
            expression = "NeoWPF::UI::ResponsiveHover::Off";
            return true;
        }
        return false;
    }
};

class KeyboardConverter final : public NeoWPF::UI::Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        NeoWPF::UI::ResponsiveKeyboard keyboard{};
        if (!ParseKeyboard(rawValue, keyboard)) {
            return false;
        }
        convertedValue = keyboard;
        return true;
    }

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override
    {
        NeoWPF::UI::ResponsiveKeyboard keyboard{};
        if (!ParseKeyboard(rawValue, keyboard)) {
            return false;
        }
        switch (keyboard) {
        case NeoWPF::UI::ResponsiveKeyboard::Any:
            expression = "NeoWPF::UI::ResponsiveKeyboard::Any";
            return true;
        case NeoWPF::UI::ResponsiveKeyboard::Shown:
            expression = "NeoWPF::UI::ResponsiveKeyboard::Shown";
            return true;
        case NeoWPF::UI::ResponsiveKeyboard::Hidden:
            expression = "NeoWPF::UI::ResponsiveKeyboard::Hidden";
            return true;
        }
        return false;
    }
};

}

namespace NeoWPF::UI::Properties {

std::shared_ptr<const TypeConverter> GetResponsiveScopeConverter()
{
    static const auto converter = std::make_shared<ScopeConverter>();
    return converter;
}

std::shared_ptr<const TypeConverter> GetResponsivePointerConverter()
{
    static const auto converter = std::make_shared<PointerConverter>();
    return converter;
}

std::shared_ptr<const TypeConverter> GetResponsiveHoverConverter()
{
    static const auto converter = std::make_shared<HoverConverter>();
    return converter;
}

std::shared_ptr<const TypeConverter> GetResponsiveKeyboardConverter()
{
    static const auto converter = std::make_shared<KeyboardConverter>();
    return converter;
}

}
