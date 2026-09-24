#include <NeoWPF/UI/Controls/Clipboard/Clipboard.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

#include <utility>

namespace NeoWPF::UI {

bool Clipboard::SetText(const std::wstring& text)
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform && platform->Clipboard().SetText(text);
}

void Clipboard::TryGetText(TextCallback callback)
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        auto& appState = NeoWPF::Core::GetAppState();
        if (callback && appState.mainThreadExecutor) {
            appState.mainThreadExecutor->Post([callback = std::move(callback)]() mutable {
                callback(false, {});
            });
        }
        return;
    }

    platform->Clipboard().TryGetText([callback = std::move(callback)](
                                          NeoWPF::Contracts::ClipboardTextResult result) {
        if (callback) {
            callback(result.Succeeded, std::move(result.Text));
        }
    });
}

bool Clipboard::Clear()
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    return platform && platform->Clipboard().Clear();
}

}