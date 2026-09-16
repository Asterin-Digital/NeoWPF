#include <NeoWPF/UI/Controls/FilePicker/FilePicker.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeFilePickerTitleMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFilePickerDirectoryMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFilePickerFileNameMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFilePickerSelectedFileMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::wstring());
        metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFilePickerAllowMultiSelectMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(false);
        metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterFilePickerProperties() {
        RegisterProperty<UI::FilePicker, std::wstring>("Title",
            &UI::FilePicker::GetTitle,
            &UI::FilePicker::SetTitle,
            MakeFilePickerTitleMetadata());
        RegisterProperty<UI::FilePicker, std::wstring>("InitialDirectory",
            &UI::FilePicker::GetInitialDirectory,
            &UI::FilePicker::SetInitialDirectory,
            MakeFilePickerDirectoryMetadata());
        RegisterProperty<UI::FilePicker, std::wstring>("DefaultFileName",
            &UI::FilePicker::GetDefaultFileName,
            &UI::FilePicker::SetDefaultFileName,
            MakeFilePickerFileNameMetadata());
        RegisterProperty<UI::FilePicker, std::wstring>("SelectedFile",
            &UI::FilePicker::GetSelectedFile,
            &UI::FilePicker::SetSelectedFile,
            MakeFilePickerSelectedFileMetadata());
        RegisterProperty<UI::FilePicker, bool>("AllowMultiSelect",
            &UI::FilePicker::GetAllowMultiSelect,
            &UI::FilePicker::SetAllowMultiSelect,
            MakeFilePickerAllowMultiSelectMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::FilePicker>(
            "FilePicker", "", "NeoWPF::UI::FilePicker", "NeoWPF/UI/Controls/FilePicker/FilePicker.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::FilePicker::TitleProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::FilePicker, std::wstring>(
        "Title",
        &NeoWPF::UI::FilePicker::GetTitle,
        &NeoWPF::UI::FilePicker::SetTitle,
        MakeFilePickerTitleMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::FilePicker::InitialDirectoryProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::FilePicker, std::wstring>(
        "InitialDirectory",
        &NeoWPF::UI::FilePicker::GetInitialDirectory,
        &NeoWPF::UI::FilePicker::SetInitialDirectory,
        MakeFilePickerDirectoryMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::FilePicker::DefaultFileNameProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::FilePicker, std::wstring>(
        "DefaultFileName",
        &NeoWPF::UI::FilePicker::GetDefaultFileName,
        &NeoWPF::UI::FilePicker::SetDefaultFileName,
        MakeFilePickerFileNameMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::FilePicker::SelectedFileProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::FilePicker, std::wstring>(
        "SelectedFile",
        &NeoWPF::UI::FilePicker::GetSelectedFile,
        &NeoWPF::UI::FilePicker::SetSelectedFile,
        MakeFilePickerSelectedFileMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::FilePicker::AllowMultiSelectProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::FilePicker, bool>(
        "AllowMultiSelect",
        &NeoWPF::UI::FilePicker::GetAllowMultiSelect,
        &NeoWPF::UI::FilePicker::SetAllowMultiSelect,
        MakeFilePickerAllowMultiSelectMetadata());
    return *property;
}
