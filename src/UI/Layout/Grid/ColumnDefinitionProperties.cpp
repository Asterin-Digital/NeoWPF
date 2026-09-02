#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeColumnDefinitionWidthMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Grid::GridLength::Auto());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridLengthConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterColumnDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::ColumnDefinition, NeoWPF::UI::Grid::GridLength>("Width", MakeColumnDefinitionWidthMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::WidthProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::ColumnDefinition, NeoWPF::UI::Grid::GridLength>("Width", MakeColumnDefinitionWidthMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::ModeProperty() { return WidthProperty(); }

void NeoWPF::UI::ColumnDefinition::SetWidth(NeoWPF::UI::Grid::GridLength width) {
    SetValue(WidthProperty(), width);
}

NeoWPF::UI::Grid::GridLength NeoWPF::UI::ColumnDefinition::GetWidth() const {
    return GetValue<NeoWPF::UI::Grid::GridLength>(WidthProperty());
}

void NeoWPF::UI::ColumnDefinition::SetMode(int mode) {
    SetWidth({static_cast<NeoWPF::UI::SizeMode>(mode), 0});
}

int NeoWPF::UI::ColumnDefinition::GetMode() const {
    return static_cast<int>(GetWidth().Mode);
}
