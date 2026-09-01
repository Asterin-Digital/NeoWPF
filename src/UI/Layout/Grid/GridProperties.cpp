#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>
#include <NeoWPF/UI/Markup/Xaml/XamlPropertyBinding.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeGridRowMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender, const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
            auto* visual = dynamic_cast<NeoWPF::UI::Visual*>(sender);
            if (!visual) {
                return;
            }
            if (auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(visual->GetParent())) {
                grid->SetRow(visual, std::any_cast<int>(args.NewValue));
            }
        };
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeGridColumnMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender, const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
            auto* visual = dynamic_cast<NeoWPF::UI::Visual*>(sender);
            if (!visual) {
                return;
            }
            if (auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(visual->GetParent())) {
                grid->SetColumn(visual, std::any_cast<int>(args.NewValue));
            }
        };
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeGridRowSpanMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(1);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender, const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
            auto* visual = dynamic_cast<NeoWPF::UI::Visual*>(sender);
            if (!visual) {
                return;
            }
            if (auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(visual->GetParent())) {
                grid->SetRowSpan(visual, std::any_cast<int>(args.NewValue));
            }
        };
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeGridColumnSpanMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(1);
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender, const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
            auto* visual = dynamic_cast<NeoWPF::UI::Visual*>(sender);
            if (!visual) {
                return;
            }
            if (auto* grid = dynamic_cast<NeoWPF::UI::Grid*>(visual->GetParent())) {
                grid->SetColumnSpan(visual, std::any_cast<int>(args.NewValue));
            }
        };
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeRowDefinitionsMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any();
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeColumnDefinitionsMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterGridProperties() {
        RegisterAttachedProperty<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("Row", MakeGridRowMetadata());
        RegisterAttachedProperty<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("Column", MakeGridColumnMetadata());
        RegisterAttachedProperty<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("RowSpan", MakeGridRowSpanMetadata());
        RegisterAttachedProperty<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("ColumnSpan", MakeGridColumnSpanMetadata());

        ::NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty(
            ::NeoWPF::UI::Grid::RowDefinitionsProperty(),
            &::NeoWPF::UI::Grid::AddRowDefinition);
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty(
            ::NeoWPF::UI::Grid::ColumnDefinitionsProperty(),
            &::NeoWPF::UI::Grid::AddColumnDefinition);

        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Grid>("Grid", "Children");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::RowProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("Row", MakeGridRowMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::ColumnProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("Column", MakeGridColumnMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::RowSpanProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("RowSpan", MakeGridRowSpanMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::ColumnSpanProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<::NeoWPF::UI::Grid, ::NeoWPF::UI::Visual, int>("ColumnSpan", MakeGridColumnSpanMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::RowDefinitionsProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<::NeoWPF::UI::Grid, int>("RowDefinitions", MakeRowDefinitionsMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Grid::ColumnDefinitionsProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<::NeoWPF::UI::Grid, int>("ColumnDefinitions", MakeColumnDefinitionsMetadata());
    return property;
}
