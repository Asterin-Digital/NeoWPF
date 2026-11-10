/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IGeneral.h>

namespace NeoWPF::Platform::Windows {

class WClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    NeoWPF::Result<void> SetText(const std::wstring& text) override;
    void TryGetText(TextCallback callback) override;
    NeoWPF::Result<void> Clear() override;
};

}