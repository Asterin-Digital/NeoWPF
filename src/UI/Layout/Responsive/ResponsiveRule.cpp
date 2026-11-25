#include <NeoWPF/UI/Layout/Responsive/ResponsiveRule.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <limits>
#include <cmath>

namespace NeoWPF::UI {

namespace {

template<typename EnumType>
std::string EnumName(EnumType value)
{
    return std::to_string(static_cast<int>(value));
}

Properties::PropertyMetadata MakeDoubleMetadata(double defaultValue)
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(defaultValue);
    metadata.Converter = Properties::GetDoubleConverter();
    return metadata;
}

Properties::PropertyMetadata MakeScopeMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(ResponsiveScope::Container);
    metadata.Converter = Properties::GetResponsiveScopeConverter();
    return metadata;
}

Properties::PropertyMetadata MakePointerMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(ResponsivePointer::Any);
    metadata.Converter = Properties::GetResponsivePointerConverter();
    return metadata;
}

Properties::PropertyMetadata MakeHoverMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(ResponsiveHover::Any);
    metadata.Converter = Properties::GetResponsiveHoverConverter();
    return metadata;
}

Properties::PropertyMetadata MakeKeyboardMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(ResponsiveKeyboard::Any);
    metadata.Converter = Properties::GetResponsiveKeyboardConverter();
    return metadata;
}

Properties::PropertyMetadata MakeSettersMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue =
        std::vector<std::shared_ptr<Styling::Triggers::Setter>>{};
    return metadata;
}

}

const Properties::DependencyProperty& ResponsiveRule::MinWidthProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MinWidth", MakeDoubleMetadata(-std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MaxWidthProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MaxWidth", MakeDoubleMetadata(std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MinHeightProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MinHeight", MakeDoubleMetadata(-std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MaxHeightProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MaxHeight", MakeDoubleMetadata(std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MinAspectRatioProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MinAspectRatio", MakeDoubleMetadata(0.0));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MaxAspectRatioProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, double>(
        "MaxAspectRatio", MakeDoubleMetadata(std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::ScopeProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<ResponsiveRule, ResponsiveScope>(
        "Scope", MakeScopeMetadata());
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::PointerProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, ResponsivePointer>(
            "Pointer", MakePointerMetadata());
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::HoverProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, ResponsiveHover>(
            "Hover", MakeHoverMetadata());
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::KeyboardProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, ResponsiveKeyboard>(
            "Keyboard", MakeKeyboardMetadata());
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::SettersProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        ResponsiveRule,
        std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(
            "Setters", MakeSettersMetadata());
    return property;
}

double ResponsiveRule::GetMinWidth() const
{
    return GetValue<double>(MinWidthProperty());
}

void ResponsiveRule::SetMinWidth(double width)
{
    SetValue(MinWidthProperty(), width);
}

double ResponsiveRule::GetMaxWidth() const
{
    return GetValue<double>(MaxWidthProperty());
}

void ResponsiveRule::SetMaxWidth(double width)
{
    SetValue(MaxWidthProperty(), width);
}

double ResponsiveRule::GetMinHeight() const
{
    return GetValue<double>(MinHeightProperty());
}

void ResponsiveRule::SetMinHeight(double height)
{
    SetValue(MinHeightProperty(), height);
}

double ResponsiveRule::GetMaxHeight() const
{
    return GetValue<double>(MaxHeightProperty());
}

void ResponsiveRule::SetMaxHeight(double height)
{
    SetValue(MaxHeightProperty(), height);
}

double ResponsiveRule::GetMinAspectRatio() const
{
    return GetValue<double>(MinAspectRatioProperty());
}

void ResponsiveRule::SetMinAspectRatio(double ratio)
{
    SetValue(MinAspectRatioProperty(), ratio);
}

double ResponsiveRule::GetMaxAspectRatio() const
{
    return GetValue<double>(MaxAspectRatioProperty());
}

void ResponsiveRule::SetMaxAspectRatio(double ratio)
{
    SetValue(MaxAspectRatioProperty(), ratio);
}

ResponsiveScope ResponsiveRule::GetScope() const
{
    return GetValue<ResponsiveScope>(ScopeProperty());
}

void ResponsiveRule::SetScope(ResponsiveScope scope)
{
    SetValue(ScopeProperty(), scope);
}

ResponsivePointer ResponsiveRule::GetPointer() const
{
    return GetValue<ResponsivePointer>(PointerProperty());
}

void ResponsiveRule::SetPointer(ResponsivePointer pointer)
{
    SetValue(PointerProperty(), pointer);
}

ResponsiveHover ResponsiveRule::GetHover() const
{
    return GetValue<ResponsiveHover>(HoverProperty());
}

void ResponsiveRule::SetHover(ResponsiveHover hover)
{
    SetValue(HoverProperty(), hover);
}

ResponsiveKeyboard ResponsiveRule::GetKeyboard() const
{
    return GetValue<ResponsiveKeyboard>(KeyboardProperty());
}

void ResponsiveRule::SetKeyboard(ResponsiveKeyboard keyboard)
{
    SetValue(KeyboardProperty(), keyboard);
}

std::vector<std::shared_ptr<Styling::Triggers::Setter>>
ResponsiveRule::GetSetters() const
{
    return GetValue<std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(
        SettersProperty());
}

void ResponsiveRule::AddSetter(std::shared_ptr<Styling::Triggers::Setter> setter)
{
    if (!setter) {
        return;
    }
    auto setters = GetSetters();
    setters.push_back(std::move(setter));
    SetValue(SettersProperty(), setters);
}

void ResponsiveRule::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    (void)property;
    (void)metadata;
    (void)oldValue;
    (void)newValue;
    (void)source;
    m_changed->Publish();
}

bool ResponsiveRule::Matches(double width, double height) const
{
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0.0 || height <= 0.0) {
        return false;
    }
    const double ratio = width / height;
    return width >= GetMinWidth() && width < GetMaxWidth() &&
           height >= GetMinHeight() && height < GetMaxHeight() &&
           ratio >= GetMinAspectRatio() && ratio < GetMaxAspectRatio();
}

bool ResponsiveRule::Matches(double width,
                             double height,
                             NeoWPF::UI::Layout::PointerPrecision pointer,
                             bool hover,
                             bool keyboardShown) const
{
    if (!Matches(width, height)) {
        return false;
    }

    const bool pointerMatches =
        GetPointer() == ResponsivePointer::Any ||
        (GetPointer() == ResponsivePointer::Fine &&
         pointer == NeoWPF::UI::Layout::PointerPrecision::Fine) ||
        (GetPointer() == ResponsivePointer::Coarse &&
         pointer == NeoWPF::UI::Layout::PointerPrecision::Coarse);
    const bool hoverMatches =
        GetHover() == ResponsiveHover::Any ||
        (GetHover() == ResponsiveHover::On && hover) ||
        (GetHover() == ResponsiveHover::Off && !hover);
    const bool keyboardMatches =
        GetKeyboard() == ResponsiveKeyboard::Any ||
        (GetKeyboard() == ResponsiveKeyboard::Shown && keyboardShown) ||
        (GetKeyboard() == ResponsiveKeyboard::Hidden && !keyboardShown);

    return pointerMatches && hoverMatches && keyboardMatches;
}

std::size_t ResponsiveRule::AddChangedHandler(std::function<void()> handler)
{
    return m_changed->Subscribe(std::move(handler));
}

void ResponsiveRule::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed->Unsubscribe(handlerId);
}

}
