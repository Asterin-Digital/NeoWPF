#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Styling/Style/DefaultStyles.h>

namespace NeoWPF::Core {

void DumpVisualTreeImpl(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options)
{
    const auto& registeredVisuals = NeoWPF::UI::Visual::GetRegisteredVisuals();
    if (registeredVisuals.empty()) {
        NeoWPF::Diagnostics::LogWarn("No visual tree to dump - no registered root visuals");
        return;
    }

    for (auto rootVisual : registeredVisuals) {
        if (!rootVisual->GetParent()) {
            NeoWPF::Diagnostics::VisualTreeDumper::DumpTree(rootVisual, options);
        }
    }
}

}
