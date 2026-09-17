#include <NeoPM/UI/Components/Dialog.h>

namespace NeoPM::UI::Components {

NeoWPF::Ref<NeoWPF::UI::Popup> MakeDialog(
    NeoWPF::UI::FrameworkElement* placementTarget,
    const NeoWPF::Ref<NeoWPF::UI::UIElement>& content,
    NeoWPF::UI::PopupPlacement placement)
{
    auto dialog = NeoWPF::New<NeoWPF::UI::Popup>();
    dialog->SetPlacementTarget(placementTarget);
    dialog->SetPlacement(placement);
    dialog->SetStaysOpen(false);
    dialog->SetContent(content);
    dialog->SetIsOpen(true);
    return dialog;
}

}
