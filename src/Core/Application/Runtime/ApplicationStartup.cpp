#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

namespace NeoWPF::Core {

void DumpVisualTreeImpl(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options)
{
    auto& appState = GetAppState();
    if (appState.windows.empty()) {
        NeoWPF::Diagnostics::LogWarn("No visual tree to dump, no windows have content attached");
        return;
    }

    for (const auto& windowState : appState.windows) {
        if (windowState.windowHost && windowState.windowHost->GetContent()) {
            auto rootVisual = windowState.windowHost->GetContent();
            rootVisual->Initialize();
            NeoWPF::Diagnostics::VisualTreeDumper::DumpTree(rootVisual.get(), options);
        }
    }
}

}
