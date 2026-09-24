/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IGeneral.h>

namespace NeoWPF::Platform::Windows {

class WClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    bool SetText(const std::wstring& text) override;
    void TryGetText(TextCallback callback) override;
    bool Clear() override;
};

}