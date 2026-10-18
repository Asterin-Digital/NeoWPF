/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <vector>

#include <NeoWPF/Contracts/IGeneral.h>
#include <NeoWPF/UI/Controls/Button/Button.h>

namespace NeoWPF::UI {

enum class FilePickerMode {
    Open,
    Save
};

class FilePicker : public Button {
public:
    static const Properties::DependencyProperty& TitleProperty();
    static const Properties::DependencyProperty& InitialDirectoryProperty();
    static const Properties::DependencyProperty& DefaultFileNameProperty();
    static const Properties::DependencyProperty& SelectedFileProperty();
    static const Properties::DependencyProperty& AllowMultiSelectProperty();

    FilePicker();
    explicit FilePicker(const std::wstring& label);
    ~FilePicker();

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const noexcept { return m_title; }

    void SetInitialDirectory(const std::wstring& directory);
    const std::wstring& GetInitialDirectory() const noexcept { return m_initialDirectory; }

    void SetDefaultFileName(const std::wstring& fileName);
    const std::wstring& GetDefaultFileName() const noexcept { return m_defaultFileName; }

    void SetSelectedFile(const std::wstring& fileName);
    const std::wstring& GetSelectedFile() const noexcept { return m_selectedFile; }

    void SetAllowMultiSelect(bool value) noexcept;
    bool GetAllowMultiSelect() const noexcept { return m_allowMultiSelect; }

    void SetDialogMode(FilePickerMode mode) noexcept;
    FilePickerMode GetDialogMode() const noexcept { return m_dialogMode; }

    void SetFilters(const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters);
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& GetFilters() const noexcept { return m_filters; }

    NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog();
    NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog();
    NeoWPF::Contracts::FileDialogResult ShowDialog();

    NeoWPF::Core::Event<const NeoWPF::Contracts::FileDialogResult&> FilePicked;

private:
    std::wstring m_title;
    std::wstring m_initialDirectory;
    std::wstring m_defaultFileName;
    std::wstring m_selectedFile;
    std::vector<std::wstring> m_selectedFiles;
    std::vector<NeoWPF::Contracts::FileDialogFilter> m_filters;
    bool m_allowMultiSelect = false;
    FilePickerMode m_dialogMode = FilePickerMode::Open;
};

}
