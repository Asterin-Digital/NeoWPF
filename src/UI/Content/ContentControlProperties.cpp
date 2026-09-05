#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace NeoWPF::UI::Properties {

void RegisterContentControlProperties()
{
    (void)NeoWPF::UI::Content::ContentControl::ContentProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::UI::Content::ContentControl>(
        "ContentControl",
        "Content",
        "NeoWPF::UI::Content::ContentControl",
        "NeoWPF/UI/Content/ContentControl.h");
}

}