#include <NeoWPF/UI/Layout/UniformGrid/UniformGrid.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeUniformGridDimensionMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(0);
    metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterUniformGridProperties()
{
    RegisterProperty<UI::UniformGrid, int>("Rows", MakeUniformGridDimensionMetadata());
    RegisterProperty<UI::UniformGrid, int>("Columns", MakeUniformGridDimensionMetadata());
    RegisterProperty<UI::UniformGrid, int>("FirstColumn", MakeUniformGridDimensionMetadata());

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::UniformGrid>(
        "UniformGrid", "Children", "NeoWPF::UI::UniformGrid", "NeoWPF/UI/Layout/UniformGrid/UniformGrid.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::UniformGrid::RowsProperty()
{
    return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::UniformGrid, int>(
        "Rows", MakeUniformGridDimensionMetadata());
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::UniformGrid::ColumnsProperty()
{
    return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::UniformGrid, int>(
        "Columns", MakeUniformGridDimensionMetadata());
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::UniformGrid::FirstColumnProperty()
{
    return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::UniformGrid, int>(
        "FirstColumn", MakeUniformGridDimensionMetadata());
}