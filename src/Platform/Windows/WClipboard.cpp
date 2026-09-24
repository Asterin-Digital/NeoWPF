#include <NeoWPF/Platform/Windows/WClipboard.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>

#include <windows.h>

#include <cstring>
#include <utility>

namespace NeoWPF::Platform::Windows {

bool WClipboard::SetText(const std::wstring& text)
{
    if (!OpenClipboard(nullptr)) {
        return false;
    }

    const auto closeClipboard = [] {
        CloseClipboard();
    };

    if (!EmptyClipboard()) {
        closeClipboard();
        return false;
    }

    const auto byteCount = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if (!memory) {
        closeClipboard();
        return false;
    }

    void* data = GlobalLock(memory);
    if (!data) {
        GlobalFree(memory);
        closeClipboard();
        return false;
    }

    std::memcpy(data, text.c_str(), byteCount);
    GlobalUnlock(memory);

    if (!SetClipboardData(CF_UNICODETEXT, memory)) {
        GlobalFree(memory);
        closeClipboard();
        return false;
    }

    closeClipboard();
    return true;
}

void WClipboard::TryGetText(TextCallback callback)
{
    NeoWPF::Contracts::ClipboardTextResult result;

    if (OpenClipboard(nullptr)) {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            if (const HGLOBAL memory = static_cast<HGLOBAL>(GetClipboardData(CF_UNICODETEXT))) {
                if (const auto* data = static_cast<const wchar_t*>(GlobalLock(memory))) {
                    result.Text.assign(data);
                    result.Succeeded = true;
                    GlobalUnlock(memory);
                }
            }
        }
        CloseClipboard();
    }

    auto& appState = NeoWPF::Core::GetAppState();
    if (callback && appState.mainThreadExecutor) {
        appState.mainThreadExecutor->Post([callback = std::move(callback), result = std::move(result)]() mutable {
            callback(std::move(result));
        });
    }
}

bool WClipboard::Clear()
{
    if (!OpenClipboard(nullptr)) {
        return false;
    }

    const bool cleared = EmptyClipboard() != FALSE;
    CloseClipboard();
    return cleared;
}

}