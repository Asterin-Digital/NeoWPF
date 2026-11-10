#include <NeoWPF/Platform/Services/Clipboard.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

#include <utility>

namespace NeoWPF::Platform::Services {

NeoWPF::Result<void> Clipboard::SetText(const std::wstring& text)
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Fail<std::string>{"No platform is registered for clipboard access"};
    }
    return platform->Clipboard().SetText(text);
}

void Clipboard::TryGetText(TextCallback callback)
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        auto& appState = NeoWPF::Core::GetAppState();
        if (callback && appState.mainThreadExecutor) {
            appState.mainThreadExecutor->Post([callback = std::move(callback)]() mutable {
                callback(NeoWPF::Fail<std::string>{"No platform is registered for clipboard access"});
            });
        }
        return;
    }

    platform->Clipboard().TryGetText([callback = std::move(callback)](
                                          NeoWPF::Contracts::ClipboardTextResult result) {
        if (callback) {
            callback(std::move(result));
        }
    });
}

NeoWPF::Result<void> Clipboard::Clear()
{
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Fail<std::string>{"No platform is registered for clipboard access"};
    }
    return platform->Clipboard().Clear();
}

}
