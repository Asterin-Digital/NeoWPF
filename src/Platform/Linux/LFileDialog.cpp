#include <NeoWPF/Platform/Linux/LFileDialog.h>

#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace NeoWPF::Platform::Linux {
namespace {

std::wstring EscapeForShell(const std::wstring& value)
{
    std::wstring escaped;
    escaped.reserve(value.size() + 2);
    escaped.push_back(L'\'');
    for (wchar_t character : value) {
        if (character == L'\'') {
            escaped += L"'\"'\"'";
        } else {
            escaped.push_back(character);
        }
    }
    escaped.push_back(L'\'');
    return escaped;
}

std::string CommandExists(const std::string& command)
{
    std::string probe = "command -v " + command + " >/dev/null 2>&1";
    return std::system(probe.c_str()) == 0 ? command : std::string();
}

std::string CaptureCommandOutput(const std::string& command)
{
    std::array<char, 4096> buffer{};
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return {};
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }

    const auto result = pclose(pipe);
    if (result == -1) {
        return {};
    }

    return output;
}

std::vector<std::wstring> SplitSelectedValues(const std::wstring& value)
{
    std::vector<std::wstring> parts;
    std::wstring current;
    for (const auto ch : value) {
        if (ch == L'\n' || ch == L'\r' || ch == L'|') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

std::wstring BuildFilterPattern(const std::vector<std::wstring>& extensions)
{
    std::wstring pattern;
    for (const auto& extension : extensions) {
        const auto normalized = NeoWPF::Platform::NormalizeFileExtension(extension);
        if (normalized.empty()) {
            continue;
        }

        std::wstring entry = normalized;
        if (entry.front() != L'*') {
            entry = L"*" + entry;
        }

        if (pattern.empty()) {
            pattern = entry;
        } else {
            pattern += L" " + entry;
        }
    }
    if (pattern.empty()) {
        pattern = L"*";
    }
    return pattern;
}

std::wstring MakeFilterEntry(const NeoWPF::Contracts::FileDialogFilter& filter)
{
    std::wstring name = filter.Name.empty() ? L"Files" : filter.Name;
    std::wstring pattern = BuildFilterPattern(filter.Extensions);
    return name + L" | " + pattern;
}

NeoWPF::Result<std::string> BuildCommonCommandPrefix(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::wstring& defaultFileName,
    bool saveDialog,
    bool allowMultiSelect,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    std::string command = "zenity";
    if (saveDialog) {
        command += " --file-selection --save --confirm-overwrite";
    } else {
        command += " --file-selection";
        if (allowMultiSelect) {
            command += " --multiple";
        }
    }

    if (!title.empty()) {
        auto titleUtf8 = NeoWPF::Platform::WideToUtf8(EscapeForShell(title));
        if (titleUtf8.Fail()) {
            return NeoWPF::Fail<std::string>{titleUtf8.Error()};
        }
        command += " --title=" + std::move(titleUtf8).Value();
    }

    std::wstring startPath;
    if (!initialDirectory.empty()) {
        startPath = initialDirectory;
        if (saveDialog && !defaultFileName.empty()) {
            startPath = (std::filesystem::path(initialDirectory) / std::filesystem::path(defaultFileName)).wstring();
        }
    } else if (saveDialog && !defaultFileName.empty()) {
        startPath = defaultFileName;
    }

    if (!startPath.empty()) {
        auto startPathUtf8 = NeoWPF::Platform::WideToUtf8(EscapeForShell(startPath));
        if (startPathUtf8.Fail()) {
            return NeoWPF::Fail<std::string>{startPathUtf8.Error()};
        }
        command += " --filename=" + std::move(startPathUtf8).Value();
    }

    if (!filters.empty()) {
        for (const auto& filter : filters) {
            const auto filterEntry = MakeFilterEntry(filter);
            auto filterUtf8 = NeoWPF::Platform::WideToUtf8(EscapeForShell(filterEntry));
            if (filterUtf8.Fail()) {
                return NeoWPF::Fail<std::string>{filterUtf8.Error()};
            }
            command += " --file-filter=" + std::move(filterUtf8).Value();
        }
    }

    command += " --separator='|'";
    return NeoWPF::Success<std::string>{std::move(command)};
}

NeoWPF::Contracts::FileDialogResult MakeResultFromOutput(const std::wstring& rawOutput)
{
    const auto paths = SplitSelectedValues(rawOutput);
    if (paths.empty()) {
        return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
    }

    return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{
        NeoWPF::Contracts::FileDialogSelection{paths.front(), paths}};
}

NeoWPF::Contracts::FileDialogResult TryZenityDialog(const std::wstring& title,
                                                  const std::wstring& initialDirectory,
                                                  const std::wstring& defaultFileName,
                                                  const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
                                                  bool saveDialog,
                                                  bool allowMultiSelect)
{
    if (std::system("command -v zenity >/dev/null 2>&1") != 0) {
        return NeoWPF::Fail<std::string>{"Zenity is not installed"};
    }

    auto command = BuildCommonCommandPrefix(
        title, initialDirectory, defaultFileName, saveDialog, allowMultiSelect, filters);
    if (command.Fail()) {
        return NeoWPF::Fail<std::string>{command.Error()};
    }
    const auto output = CaptureCommandOutput(command.Value());
    if (output.empty()) {
        return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
    }

    auto wideOutput = NeoWPF::Platform::Utf8ToWide(output);
    if (wideOutput.Fail()) {
        return NeoWPF::Fail<std::string>{wideOutput.Error()};
    }
    return MakeResultFromOutput(std::move(wideOutput).Value());
}

NeoWPF::Contracts::FileDialogResult TryKDialogDialog(const std::wstring& title,
                                                   const std::wstring& initialDirectory,
                                                   const std::wstring& defaultFileName,
                                                   const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
                                                   bool saveDialog,
                                                   bool allowMultiSelect)
{
    if (std::system("command -v kdialog >/dev/null 2>&1") != 0) {
        return NeoWPF::Fail<std::string>{"KDialog is not installed"};
    }

    std::string command = "kdialog";
    if (saveDialog) {
        command += " --getsavefilename";
    } else {
        command += " --getopenfilename";
        if (allowMultiSelect) {
            command += " --multiple";
        }
    }

    if (!initialDirectory.empty()) {
        auto directoryUtf8 = NeoWPF::Platform::WideToUtf8(initialDirectory);
        if (directoryUtf8.Fail()) {
            return NeoWPF::Fail<std::string>{directoryUtf8.Error()};
        }
        command += " '" + std::move(directoryUtf8).Value() + "'";
    }

    if (!filters.empty()) {
        std::string filterList;
        for (const auto& filter : filters) {
            const auto filterEntry = MakeFilterEntry(filter);
            if (!filterList.empty()) {
                filterList += " ";
            }
            auto filterUtf8 = NeoWPF::Platform::WideToUtf8(filterEntry);
            if (filterUtf8.Fail()) {
                return NeoWPF::Fail<std::string>{filterUtf8.Error()};
            }
            filterList += "'" + std::move(filterUtf8).Value() + "'";
        }
        if (!filterList.empty()) {
            command += " " + filterList;
        }
    }

    if (!title.empty()) {
        auto titleUtf8 = NeoWPF::Platform::WideToUtf8(title);
        if (titleUtf8.Fail()) {
            return NeoWPF::Fail<std::string>{titleUtf8.Error()};
        }
        command += " --title '" + std::move(titleUtf8).Value() + "'";
    }

    if (saveDialog && !defaultFileName.empty()) {
        auto fileNameUtf8 = NeoWPF::Platform::WideToUtf8(defaultFileName);
        if (fileNameUtf8.Fail()) {
            return NeoWPF::Fail<std::string>{fileNameUtf8.Error()};
        }
        command += " '" + std::move(fileNameUtf8).Value() + "'";
    }

    const auto output = CaptureCommandOutput(command);
    if (output.empty()) {
        return NeoWPF::Success<std::optional<NeoWPF::Contracts::FileDialogSelection>>{std::nullopt};
    }

    auto wideOutput = NeoWPF::Platform::Utf8ToWide(output);
    if (wideOutput.Fail()) {
        return NeoWPF::Fail<std::string>{wideOutput.Error()};
    }
    return MakeResultFromOutput(std::move(wideOutput).Value());
}

}

std::vector<NeoWPF::Contracts::FileDialogFilter> LFileDialog::BuildFilterSpecs(
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

NeoWPF::Contracts::FileDialogResult LFileDialog::ShowOpenFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
    bool allowMultiSelect)
{
    const auto normalizedFilters = BuildFilterSpecs(filters);

    auto result = TryZenityDialog(title, initialDirectory, {}, normalizedFilters, false, allowMultiSelect);
    if (result.Success()) {
        return result;
    }

    auto fallback = TryKDialogDialog(title, initialDirectory, {}, normalizedFilters, false, allowMultiSelect);
    if (fallback.Success()) {
        return fallback;
    }

    return NeoWPF::Fail<std::string>{result.Error() + "; " + fallback.Error()};
}

NeoWPF::Contracts::FileDialogResult LFileDialog::ShowSaveFileDialog(
    const std::wstring& title,
    const std::wstring& initialDirectory,
    const std::wstring& defaultFileName,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters)
{
    const auto normalizedFilters = BuildFilterSpecs(filters);

    auto result = TryZenityDialog(title, initialDirectory, defaultFileName, normalizedFilters, true, false);
    if (result.Success()) {
        return result;
    }

    auto fallback = TryKDialogDialog(title, initialDirectory, defaultFileName, normalizedFilters, true, false);
    if (fallback.Success()) {
        return fallback;
    }

    return NeoWPF::Fail<std::string>{result.Error() + "; " + fallback.Error()};
}

}
