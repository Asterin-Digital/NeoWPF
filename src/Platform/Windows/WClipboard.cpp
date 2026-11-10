#include <NeoWPF/Platform/Windows/WClipboard.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>

#include <windows.h>

#include <cstring>
#include <utility>

namespace NeoWPF::Platform::Windows {

NeoWPF::Result<void> WClipboard::SetText(const std::wstring& text)
{
    if (!OpenClipboard(nullptr)) {
        return NeoWPF::Fail<std::string>{"Windows clipboard could not be opened"};
    }

    const auto closeClipboard = [] {
        CloseClipboard();
    };

    if (!EmptyClipboard()) {
        closeClipboard();
        return NeoWPF::Fail<std::string>{"Windows clipboard could not be cleared before writing"};
    }

    const auto byteCount = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if (!memory) {
        closeClipboard();
        return NeoWPF::Fail<std::string>{"Windows clipboard text storage could not be allocated"};
    }

    void* data = GlobalLock(memory);
    if (!data) {
        GlobalFree(memory);
        closeClipboard();
        return NeoWPF::Fail<std::string>{"Windows clipboard text storage could not be locked"};
    }

    std::memcpy(data, text.c_str(), byteCount);
    GlobalUnlock(memory);

    if (!SetClipboardData(CF_UNICODETEXT, memory)) {
        GlobalFree(memory);
        closeClipboard();
        return NeoWPF::Fail<std::string>{"Windows clipboard text could not be written"};
    }

    closeClipboard();
    return NeoWPF::Success<void>{};
}

void WClipboard::TryGetText(TextCallback callback)
{
    NeoWPF::Contracts::ClipboardTextResult result =
        NeoWPF::Success<std::optional<std::wstring>>{std::nullopt};

    if (!OpenClipboard(nullptr)) {
        result = NeoWPF::Fail<std::string>{"Windows clipboard could not be opened for reading"};
    } else {
        if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            const HGLOBAL memory = static_cast<HGLOBAL>(GetClipboardData(CF_UNICODETEXT));
            if (!memory) {
                result = NeoWPF::Fail<std::string>{"Windows clipboard text could not be accessed"};
            } else if (const auto* data = static_cast<const wchar_t*>(GlobalLock(memory))) {
                result = NeoWPF::Success<std::optional<std::wstring>>{std::wstring(data)};
                GlobalUnlock(memory);
            } else {
                result = NeoWPF::Fail<std::string>{"Windows clipboard text could not be locked"};
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

NeoWPF::Result<void> WClipboard::Clear()
{
    if (!OpenClipboard(nullptr)) {
        return NeoWPF::Fail<std::string>{"Windows clipboard could not be opened"};
    }

    const bool cleared = EmptyClipboard() != FALSE;
    CloseClipboard();
    if (!cleared) {
        return NeoWPF::Fail<std::string>{"Windows clipboard could not be cleared"};
    }
    return NeoWPF::Success<void>{};
}

}