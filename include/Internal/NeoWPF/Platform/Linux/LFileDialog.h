/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IGeneral.h>

#include <string>
#include <vector>

namespace NeoWPF::Platform::Linux {

class LFileDialog final {
public:
    static std::vector<NeoWPF::Contracts::FileDialogFilter> BuildFilterSpecs(
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters);

    static NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false);

    static NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters);
};

}
