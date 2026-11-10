#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Platform/Windows/WFileDialog.h>

#include <shobjidl.h>

#include <vector>

namespace NeoWPF::Platform::Windows {
namespace {

std::wstring BuildPatternString(const std::wstring& extension)
{
    if (extension.empty()) {
        return L"*.*";
    }
    if (extension == L"*") {
        return L"*.*";
    }
    return L"*" + extension;
}

std::wstring BuildFileDisplayName(const std::wstring& name, const std::vector<std::wstring>& extensions)
{
    std::wstring pattern;
    for (const auto& extension : extensions) {
        if (pattern.empty()) {
            pattern = BuildPatternString(extension);
        } else {
            pattern += L";" + BuildPatternString(extension);
        }
    }

    if (pattern.empty()) {
        pattern = L"*.*";
    }

    const std::wstring label = name.empty() ? L"Files" : name;
    return label + L" (" + pattern + L")";
}

void AppendResult(std::vector<std::wstring>& fileNames, IShellItem* item)
{
    if (!item) {
        return;
    }

    PWSTR filePath = nullptr;
    if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath))) {
        return;
    }

    fileNames.emplace_back(filePath);
    CoTaskMemFree(filePath);
}

NeoWPF::Contracts::FileDialogResult MakeResult(std::vector<std::wstring>&& fileNames)
{
    if (fileNames.empty()) {
        return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
    }
    return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{
        NeoWPF::Contracts::FileDialogSelection{fileNames.front(), std::move(fileNames)}};
}

}

std::vector<NeoWPF::Contracts::FileDialogFilter> WFileDialog::BuildFilterSpecs(
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    if (filters.empty()) {
        return {{L"All Files", {L"*"}}};
    }

    std::vector<NeoWPF::Contracts::FileDialogFilter> normalized;
    normalized.reserve(filters.size());

    for (const auto& filter : filters) {
        if (filter.Name.empty() && filter.Extensions.empty()) {
            continue;
        }

        NeoWPF::Contracts::FileDialogFilter normalizedFilter;
        normalizedFilter.Name = filter.Name;
        for (const auto& extension : filter.Extensions) {
            const auto normalizedExtension = NeoWPF::Platform::NormalizeFileExtension(extension);
            if (!normalizedExtension.empty()) {
                normalizedFilter.Extensions.push_back(normalizedExtension);
            }
        }

        if (normalizedFilter.Extensions.empty()) {
            if (!normalizedFilter.Name.empty()) {
                normalizedFilter.Extensions.push_back(L"*");
            }
        }

        if (!normalizedFilter.Name.empty() || !normalizedFilter.Extensions.empty()) {
            normalized.push_back(normalizedFilter);
        }
    }

    if (normalized.empty()) {
        return {{L"All Files", {L"*"}}};
    }

    return normalized;
}

NeoWPF::Contracts::FileDialogResult WFileDialog::ShowOpenFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
    bool allowMultiSelect)
{
    NEO_WPF_ACTION("FileDialog.ShowOpenFileDialog");
    const auto normalizedFilters = BuildFilterSpecs(filters);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[WFileDialog] ShowOpenFileDialog begin titleEmpty=%d initialDirEmpty=%d filterCount=%zu allowMulti=%d",
        title.empty(),
        initialDirectory.empty(),
        normalizedFilters.size(),
        allowMultiSelect);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool comInitializedHere = SUCCEEDED(hr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] CoInitializeEx failed for open dialog hr=0x%08x",
            static_cast<unsigned int>(hr));
        return NeoWPF::Fail<std::string>{"Windows could not initialize COM for the file dialog"};
    }

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] CoCreateInstance(FileOpenDialog) failed hr=0x%08x",
            static_cast<unsigned int>(hr));
        if (comInitializedHere) {
            CoUninitialize();
        }
        return NeoWPF::Fail<std::string>{"Windows could not create the open file dialog"};
    }

    std::vector<std::wstring> filterNames;
    std::vector<std::wstring> filterSpecs;
    std::vector<COMDLG_FILTERSPEC> nativeFilters;
    nativeFilters.reserve(normalizedFilters.size());

    for (const auto& filter : normalizedFilters) {
        filterNames.push_back(BuildFileDisplayName(filter.Name, filter.Extensions));
        std::wstring pattern;
        for (const auto& extension : filter.Extensions) {
            const auto patternEntry = BuildPatternString(extension);
            if (pattern.empty()) {
                pattern = patternEntry;
            } else {
                pattern += L";" + patternEntry;
            }
        }
        if (pattern.empty()) {
            pattern = L"*.*";
        }
        filterSpecs.push_back(pattern);
        nativeFilters.push_back({filterNames.back().c_str(), filterSpecs.back().c_str()});
    }

    if (!nativeFilters.empty()) {
        dialog->SetFileTypes(static_cast<UINT>(nativeFilters.size()), nativeFilters.data());
        dialog->SetFileTypeIndex(1);
    }

    if (!title.empty()) {
        dialog->SetTitle(title.c_str());
    }

    if (allowMultiSelect) {
        FILEOPENDIALOGOPTIONS options = 0;
        if (SUCCEEDED(dialog->GetOptions(&options))) {
            dialog->SetOptions(options | FOS_ALLOWMULTISELECT);
        }
    }

    if (!initialDirectory.empty()) {
        IShellItem* defaultFolder = nullptr;
        const auto wideDirectory = initialDirectory.c_str();
        if (SUCCEEDED(SHCreateItemFromParsingName(wideDirectory, nullptr, IID_PPV_ARGS(&defaultFolder)))) {
            dialog->SetDefaultFolder(defaultFolder);
            defaultFolder->Release();
        }
    }

    const auto result = dialog->Show(nullptr);
    if (FAILED(result)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] ShowOpenFileDialog user cancelled or failed hr=0x%08x",
            static_cast<unsigned int>(result));
        dialog->Release();
        if (comInitializedHere) {
            CoUninitialize();
        }
        if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
        }
        return NeoWPF::Fail<std::string>{"Windows open file dialog failed"};
    }

    IShellItemArray* items = nullptr;
    std::vector<std::wstring> selectedFiles;

    if (allowMultiSelect) {
        if (SUCCEEDED(dialog->GetResults(&items))) {
            DWORD count = 0;
            if (SUCCEEDED(items->GetCount(&count))) {
                for (DWORD index = 0; index < count; ++index) {
                    IShellItem* shellItem = nullptr;
                    if (SUCCEEDED(items->GetItemAt(index, &shellItem))) {
                        AppendResult(selectedFiles, shellItem);
                        shellItem->Release();
                    }
                }
            }
            items->Release();
        }
    } else {
        IShellItem* shellItem = nullptr;
        if (SUCCEEDED(dialog->GetResult(&shellItem))) {
            AppendResult(selectedFiles, shellItem);
            shellItem->Release();
        }
    }

    dialog->Release();
    if (comInitializedHere) {
        CoUninitialize();
    }

    return MakeResult(std::move(selectedFiles));
}

NeoWPF::Contracts::FileDialogResult WFileDialog::ShowSaveFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::wstring& defaultFileName,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    NEO_WPF_ACTION("FileDialog.ShowSaveFileDialog");
    const auto normalizedFilters = BuildFilterSpecs(filters);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[WFileDialog] ShowSaveFileDialog begin titleEmpty=%d initialDirEmpty=%d defaultFileNameEmpty=%d filterCount=%zu",
        title.empty(),
        initialDirectory.empty(),
        defaultFileName.empty(),
        normalizedFilters.size());

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool comInitializedHere = SUCCEEDED(hr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] CoInitializeEx failed for save dialog hr=0x%08x",
            static_cast<unsigned int>(hr));
        return NeoWPF::Fail<std::string>{"Windows could not initialize COM for the file dialog"};
    }

    IFileSaveDialog* dialog = nullptr;
    hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] CoCreateInstance(FileSaveDialog) failed hr=0x%08x",
            static_cast<unsigned int>(hr));
        if (comInitializedHere) {
            CoUninitialize();
        }
        return NeoWPF::Fail<std::string>{"Windows could not create the save file dialog"};
    }

    std::vector<std::wstring> filterNames;
    std::vector<std::wstring> filterSpecs;
    std::vector<COMDLG_FILTERSPEC> nativeFilters;
    nativeFilters.reserve(normalizedFilters.size());

    for (const auto& filter : normalizedFilters) {
        filterNames.push_back(BuildFileDisplayName(filter.Name, filter.Extensions));
        std::wstring pattern;
        for (const auto& extension : filter.Extensions) {
            const auto patternEntry = BuildPatternString(extension);
            if (pattern.empty()) {
                pattern = patternEntry;
            } else {
                pattern += L";" + patternEntry;
            }
        }
        if (pattern.empty()) {
            pattern = L"*.*";
        }
        filterSpecs.push_back(pattern);
        nativeFilters.push_back({filterNames.back().c_str(), filterSpecs.back().c_str()});
    }

    if (!nativeFilters.empty()) {
        dialog->SetFileTypes(static_cast<UINT>(nativeFilters.size()), nativeFilters.data());
        dialog->SetFileTypeIndex(1);
    }

    if (!title.empty()) {
        dialog->SetTitle(title.c_str());
    }

    if (!defaultFileName.empty()) {
        dialog->SetFileName(defaultFileName.c_str());
    }

    if (!initialDirectory.empty()) {
        IShellItem* defaultFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(initialDirectory.c_str(), nullptr, IID_PPV_ARGS(&defaultFolder)))) {
            dialog->SetDefaultFolder(defaultFolder);
            defaultFolder->Release();
        }
    }

    if (!normalizedFilters.empty() && !normalizedFilters.front().Extensions.empty()) {
        dialog->SetDefaultExtension(BuildPatternString(normalizedFilters.front().Extensions.front()).substr(1).c_str());
    }

    const auto result = dialog->Show(nullptr);
    if (FAILED(result)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
            NeoWPF::Diagnostics::Level::Info,
            "[WFileDialog] ShowSaveFileDialog user cancelled or failed hr=0x%08x",
            static_cast<unsigned int>(result));
        dialog->Release();
        if (comInitializedHere) {
            CoUninitialize();
        }
        if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
        }
        return NeoWPF::Fail<std::string>{"Windows save file dialog failed"};
    }

    IShellItem* shellItem = nullptr;
    std::vector<std::wstring> selectedFiles;
    if (SUCCEEDED(dialog->GetResult(&shellItem))) {
        AppendResult(selectedFiles, shellItem);
        shellItem->Release();
    }

    dialog->Release();
    if (comInitializedHere) {
        CoUninitialize();
    }

    const auto finalResult = MakeResult(std::move(selectedFiles));
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Files,
        NeoWPF::Diagnostics::Level::Info,
        "[WFileDialog] ShowSaveFileDialog complete succeeded=%d selectedCount=%zu",
        finalResult.Success() && finalResult.Value().has_value(),
        finalResult.Success() && finalResult.Value().has_value()
            ? finalResult.Value()->FileNames.size()
            : 0u);
    return finalResult;
}

}
