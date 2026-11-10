/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Contracts/IGeneral.h>
#include <wayland-client.h>

#include <string>
#include <vector>

namespace NeoWPF::Platform::Linux {

class LWindowing;

class LClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    explicit LClipboard(LWindowing& windowing) noexcept;
    ~LClipboard() override;

    LClipboard(const LClipboard&) = delete;
    LClipboard& operator=(const LClipboard&) = delete;

    NeoWPF::Result<void> SetText(const std::wstring& text) override;
    void TryGetText(TextCallback callback) override;
    NeoWPF::Result<void> Clear() override;

    void ProcessPendingReads() noexcept;
    const std::wstring& OwnedText() const noexcept { return m_ownedText; }
    void SourceCancelled(wl_data_source* source) noexcept;
    void OfferCreated(wl_data_offer* offer) noexcept;
    void OfferMimeType(wl_data_offer* offer, const char* mimeType) noexcept;
    void SelectionChanged(wl_data_offer* offer) noexcept;

private:
    bool EnsureDevice() noexcept;
    void CompleteRead(NeoWPF::Contracts::ClipboardTextResult result) noexcept;
    void ResetOffer() noexcept;

    LWindowing& m_windowing;
    wl_data_device* m_dataDevice = nullptr;
    wl_data_source* m_source = nullptr;
    wl_data_offer* m_selectionOffer = nullptr;
    wl_data_offer* m_pendingOffer = nullptr;
    std::wstring m_ownedText;
    std::vector<std::string> m_offerMimeTypes;
    int m_pendingReadFd = -1;
    std::string m_pendingReadText;
    TextCallback m_pendingCallback;
};

}