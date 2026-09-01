#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeRowDefinitionHeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(static_cast<int>(NeoWPF::UI::SizeMode::Auto));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridDefinitionModeConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterRowDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::RowDefinition, int>("Height", MakeRowDefinitionHeightMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::ModeProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::RowDefinition, int>("Height", MakeRowDefinitionHeightMetadata());
    return property;
}

void NeoWPF::UI::RowDefinition::SetMode(int mode) {
    SetValue(ModeProperty(), mode);
}

int NeoWPF::UI::RowDefinition::GetMode() const {
    return GetValue<int>(ModeProperty());
}
