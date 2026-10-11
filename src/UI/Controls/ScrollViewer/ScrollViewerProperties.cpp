#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeScrollViewerHorizontalOffsetMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0.0);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeScrollViewerVerticalOffsetMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0.0);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeScrollViewerHorizontalVisibilityMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::ScrollBarVisibility::Auto);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeScrollViewerVerticalVisibilityMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::ScrollBarVisibility::Auto);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterScrollViewerProperties() {
        RegisterProperty<UI::ScrollViewer, double>("HorizontalOffset",
            &UI::ScrollViewer::GetHorizontalOffset,
            &UI::ScrollViewer::SetHorizontalOffset,
            MakeScrollViewerHorizontalOffsetMetadata());
        RegisterProperty<UI::ScrollViewer, double>("VerticalOffset",
            &UI::ScrollViewer::GetVerticalOffset,
            &UI::ScrollViewer::SetVerticalOffset,
            MakeScrollViewerVerticalOffsetMetadata());
        RegisterProperty<UI::ScrollViewer, ::NeoWPF::UI::ScrollBarVisibility>("HorizontalScrollBarVisibility",
            &UI::ScrollViewer::GetHorizontalScrollBarVisibility,
            &UI::ScrollViewer::SetHorizontalScrollBarVisibility,
            MakeScrollViewerHorizontalVisibilityMetadata());
        RegisterProperty<UI::ScrollViewer, ::NeoWPF::UI::ScrollBarVisibility>("VerticalScrollBarVisibility",
            &UI::ScrollViewer::GetVerticalScrollBarVisibility,
            &UI::ScrollViewer::SetVerticalScrollBarVisibility,
            MakeScrollViewerVerticalVisibilityMetadata());
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::ScrollViewer,
                             HorizontalOffset,
                             double,
                             &NeoWPF::UI::ScrollViewer::GetHorizontalOffset,
                             &NeoWPF::UI::ScrollViewer::SetHorizontalOffset,
                             MakeScrollViewerHorizontalOffsetMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::ScrollViewer,
                             VerticalOffset,
                             double,
                             &NeoWPF::UI::ScrollViewer::GetVerticalOffset,
                             &NeoWPF::UI::ScrollViewer::SetVerticalOffset,
                             MakeScrollViewerVerticalOffsetMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::ScrollViewer,
                             HorizontalScrollBarVisibility,
                             NeoWPF::UI::ScrollBarVisibility,
                             &NeoWPF::UI::ScrollViewer::GetHorizontalScrollBarVisibility,
                             &NeoWPF::UI::ScrollViewer::SetHorizontalScrollBarVisibility,
                             MakeScrollViewerHorizontalVisibilityMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::ScrollViewer,
                             VerticalScrollBarVisibility,
                             NeoWPF::UI::ScrollBarVisibility,
                             &NeoWPF::UI::ScrollViewer::GetVerticalScrollBarVisibility,
                             &NeoWPF::UI::ScrollViewer::SetVerticalScrollBarVisibility,
                             MakeScrollViewerVerticalVisibilityMetadata)
