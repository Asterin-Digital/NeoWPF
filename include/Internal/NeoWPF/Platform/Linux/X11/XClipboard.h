/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IGeneral.h>
#include <NeoWPF/Platform/Linux/X11/XWindowing.h>

#include <X11/Xlib.h>

#undef Success
#undef None

#include <string>

namespace NeoWPF::Platform::Linux {

class XClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    explicit XClipboard(XWindowing& windowing) noexcept;
    ~XClipboard() override;

    NeoWPF::Result<void> SetText(const std::wstring& text) override;
    void TryGetText(TextCallback callback) override;
    NeoWPF::Result<void> Clear() override;
    void ProcessSelectionRequest(const XEvent& event) noexcept;
    void ProcessSelectionNotify(const XEvent& event) noexcept;

private:
    XWindowing& m_windowing;
    ::Window m_owner = 0;
    Atom m_clipboard = 0;
    Atom m_utf8 = 0;
    Atom m_targets = 0;
    std::wstring m_ownedText;
    TextCallback m_pendingCallback;
};

}
