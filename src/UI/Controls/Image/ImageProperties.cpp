#include <NeoWPF/UI/Controls/Image/Image.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <stdexcept>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;

void RegisterImageProperties() {
    PropertyMetadata sourceMetadata;
    sourceMetadata.DefaultValue = std::any(std::shared_ptr<::NeoWPF::UI::Core::Media::ImageSource>());
    sourceMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Image, std::shared_ptr<::NeoWPF::UI::Core::Media::ImageSource>>(
        "Source",
        &UI::Image::GetSource,
        &UI::Image::SetSource,
        sourceMetadata);

    PropertyMetadata stretchMetadata;
    stretchMetadata.DefaultValue = std::any(UI::Core::Media::Stretch::Uniform);
    stretchMetadata.Flags = PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Image, UI::Core::Media::Stretch>(
        "Stretch",
        &UI::Image::GetStretch,
        &UI::Image::SetStretch,
        stretchMetadata);

    PropertyMetadata stretchDirectionMetadata;
    stretchDirectionMetadata.DefaultValue = std::any(UI::Core::Media::StretchDirection::Both);
    stretchDirectionMetadata.Flags = PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Image, UI::Core::Media::StretchDirection>(
        "StretchDirection",
        &UI::Image::GetStretchDirection,
        &UI::Image::SetStretchDirection,
        stretchDirectionMetadata);
}

namespace {

const NeoWPF::UI::Properties::DependencyProperty& EnsureImageProperty(const char* name)
{
    NeoWPF::UI::Properties::RegisterImageProperties();
    const auto* property = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty(
        name, typeid(NeoWPF::UI::Image));
    if (!property) {
        throw std::logic_error("Image dependency property was not registered");
    }
    return *property;
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::SourceProperty = EnsureImageProperty("Source");
const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::StretchProperty = EnsureImageProperty("Stretch");
const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::StretchDirectionProperty = EnsureImageProperty("StretchDirection");

}
