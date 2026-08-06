#include <NeoWPF/UI/Controls/Image/Image.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterImageProperties() {
    PropertyMetadata sourceMetadata;
    sourceMetadata.DefaultValue = std::any(std::shared_ptr<NeoWPF::UI::Core::Media::ImageSource>());
    sourceMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Image, std::shared_ptr<UI::Core::Media::ImageSource>>(
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

}
