#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeRowDefinitionHeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Grid::GridLength::Auto());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridLengthConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterRowDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::RowDefinition, NeoWPF::UI::Grid::GridLength>("Height", MakeRowDefinitionHeightMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::HeightProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::RowDefinition, NeoWPF::UI::Grid::GridLength>("Height", MakeRowDefinitionHeightMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::ModeProperty() { return HeightProperty(); }

void NeoWPF::UI::RowDefinition::SetHeight(NeoWPF::UI::Grid::GridLength height) {
    SetValue(HeightProperty(), height);
}

NeoWPF::UI::Grid::GridLength NeoWPF::UI::RowDefinition::GetHeight() const {
    return GetValue<NeoWPF::UI::Grid::GridLength>(HeightProperty());
}

void NeoWPF::UI::RowDefinition::SetMode(int mode) {
    SetHeight({static_cast<NeoWPF::UI::SizeMode>(mode), 0});
}

int NeoWPF::UI::RowDefinition::GetMode() const {
    return static_cast<int>(GetHeight().Mode);
}
