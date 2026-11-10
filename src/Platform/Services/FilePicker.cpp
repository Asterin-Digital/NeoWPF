#include <NeoWPF/Platform/Services/FilePicker.h>

#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

namespace NeoWPF::Platform::Services {

void FilePicker::SetTitle(const std::wstring& title)
{
    m_title = title;
}

void FilePicker::SetInitialDirectory(const std::wstring& directory)
{
    m_initialDirectory = directory;
}

void FilePicker::SetDefaultFileName(const std::wstring& fileName)
{
    m_defaultFileName = fileName;
}

void FilePicker::SetAllowMultiSelect(bool value) noexcept
{
    m_allowMultiSelect = value;
}

void FilePicker::SetDialogMode(FilePickerMode mode) noexcept
{
    m_dialogMode = mode;
}

void FilePicker::SetFilters(const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    m_filters = filters;
}

NeoWPF::Contracts::FileDialogResult FilePicker::ShowOpenFileDialog()
{
    NEO_WPF_ACTION("FilePicker.ShowOpenFileDialog");

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[FilePicker] ShowOpenFileDialog ptr=%p titleEmpty=%d initDirEmpty=%d filterCount=%zu allowMulti=%d",
        this,
        m_title.empty(),
        m_initialDirectory.empty(),
        m_filters.size(),
        m_allowMultiSelect);

    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        const auto result = platform->ShowOpenFileDialog(m_title, m_initialDirectory, m_filters, m_allowMultiSelect);
        const auto hasSelection = result.Success() && result.Value().has_value();
        const auto fileCount = hasSelection ? result.Value()->FileNames.size() : 0u;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[FilePicker] ShowOpenFileDialog result ptr=%p succeeded=%d fileCount=%zu",
            this,
            hasSelection,
            fileCount);
        FilePicked.Invoke(result);
        return result;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[FilePicker] ShowOpenFileDialog missing platform ptr=%p",
        this);
    return NeoWPF::Fail<std::string>{"No platform is registered for file dialogs"};
}

NeoWPF::Contracts::FileDialogResult FilePicker::ShowSaveFileDialog()
{
    NEO_WPF_ACTION("FilePicker.ShowSaveFileDialog");

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[FilePicker] ShowSaveFileDialog ptr=%p titleEmpty=%d initDirEmpty=%d defaultFileNameEmpty=%d filterCount=%zu",
        this,
        m_title.empty(),
        m_initialDirectory.empty(),
        m_defaultFileName.empty(),
        m_filters.size());

    if (auto* platform = NeoWPF::Platform::CurrentPlatform()) {
        const auto result = platform->ShowSaveFileDialog(m_title, m_initialDirectory, m_defaultFileName, m_filters);
        const auto hasSelection = result.Success() && result.Value().has_value();
        const auto fileCount = hasSelection ? result.Value()->FileNames.size() : 0u;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[FilePicker] ShowSaveFileDialog result ptr=%p succeeded=%d fileCount=%zu",
            this,
            hasSelection,
            fileCount);
        FilePicked.Invoke(result);
        return result;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[FilePicker] ShowSaveFileDialog missing platform ptr=%p",
        this);
    return NeoWPF::Fail<std::string>{"No platform is registered for file dialogs"};
}

NeoWPF::Contracts::FileDialogResult FilePicker::ShowDialog()
{
    NEO_WPF_ACTION("FilePicker.ShowDialog");

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Debug,
        "[FilePicker] ShowDialog ptr=%p mode=%d",
        this,
        static_cast<int>(m_dialogMode));

    if (m_dialogMode == FilePickerMode::Save) {
        return ShowSaveFileDialog();
    }
    return ShowOpenFileDialog();
}

}
