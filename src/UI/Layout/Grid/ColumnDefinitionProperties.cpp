#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeColumnModeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(static_cast<int>(NeoWPF::UI::SizeMode::Auto));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterColumnDefinitionProperties() {
        RegisterProperty<::NeoWPF::UI::ColumnDefinition, int>("Mode", MakeColumnModeMetadata());


        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::ColumnDefinition>("ColumnDefinition");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::ModeProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<::NeoWPF::UI::ColumnDefinition, int>("Mode", MakeColumnModeMetadata());
    return property;
}

void NeoWPF::UI::ColumnDefinition::SetMode(int mode) {
    SetValue(ModeProperty(), std::any(mode));
}

int NeoWPF::UI::ColumnDefinition::GetMode() const {
    return GetValue<int>(ModeProperty());
}
