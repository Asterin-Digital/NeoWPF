/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IFileDialog.h>

#include <string>
#include <vector>

namespace NeoWPF::Platform::Windows {

class WFileDialog final {
public:
    /*
        @summary
        Normalizes a filter list for the native Windows dialog.
    */
    static std::vector<NeoWPF::Contracts::FileDialogFilter> BuildFilterSpecs(
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters);

    /*
        @summary
        Shows the native Windows open dialog.
    */
    static NeoWPF::Contracts::FileDialogResult ShowOpenFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters,
        bool allowMultiSelect = false);

    /*
        @summary
        Shows the native Windows save dialog.
    */
    static NeoWPF::Contracts::FileDialogResult ShowSaveFileDialog(
        const std::wstring& title,
        const std::wstring& initialDirectory,
        const std::wstring& defaultFileName,
        const std::vector<NeoWPF::Contracts::FileDialogFilter>& filters);
};

}
