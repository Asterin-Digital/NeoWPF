#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeRowModeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(static_cast<int>(NeoWPF::UI::SizeMode::Auto));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterRowDefinitionProperties() {
        RegisterProperty<::NeoWPF::UI::RowDefinition, int>("Mode", MakeRowModeMetadata());


        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::RowDefinition>("RowDefinition");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::ModeProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<::NeoWPF::UI::RowDefinition, int>("Mode", MakeRowModeMetadata());
    return property;
}

void NeoWPF::UI::RowDefinition::SetMode(int mode) {
    SetValue(ModeProperty(), std::any(mode));
}

int NeoWPF::UI::RowDefinition::GetMode() const {
    return GetValue<int>(ModeProperty());
}
