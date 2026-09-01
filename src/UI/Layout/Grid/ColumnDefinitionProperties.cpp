#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeColumnDefinitionWidthMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(static_cast<int>(NeoWPF::UI::SizeMode::Auto));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridDefinitionModeConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterColumnDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::ColumnDefinition, int>("Width", MakeColumnDefinitionWidthMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::ModeProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::ColumnDefinition, int>("Width", MakeColumnDefinitionWidthMetadata());
    return property;
}

void NeoWPF::UI::ColumnDefinition::SetMode(int mode) {
    SetValue(ModeProperty(), mode);
}

int NeoWPF::UI::ColumnDefinition::GetMode() const {
    return GetValue<int>(ModeProperty());
}
