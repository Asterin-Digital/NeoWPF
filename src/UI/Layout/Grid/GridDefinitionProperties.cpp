#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeGridLengthDefinitionMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Grid::GridLength::Auto());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridLengthConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::Grid, NeoWPF::UI::Grid::GridLength>("GridLength", MakeGridLengthDefinitionMetadata());
    }
}
