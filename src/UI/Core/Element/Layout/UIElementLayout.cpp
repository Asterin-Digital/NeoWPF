#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

void UIElement::InvalidateVisual() {
    m_isDirty = true;
    MarkTreeDirty();
}

void UIElement::InvalidateMeasure() {
    InvalidateVisual();
}

void UIElement::InvalidateArrange() {
    InvalidateVisual();
}

void UIElement::ClearDirty() noexcept {
    m_isDirty = false;
    ClearTreeDirty();
}

void UIElement::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                             const Properties::PropertyMetadata& metadata,
                                             const std::any& oldValue,
                                             const std::any& newValue,
                                             Properties::PropertyValueSource source)
{
    if (HasFlag(metadata.Flags, Properties::PropertyFlags::AffectsMeasure)) {
        InvalidateMeasure();
        return;
    }

    if (HasFlag(metadata.Flags, Properties::PropertyFlags::AffectsArrange)) {
        InvalidateArrange();
        return;
    }

    if (HasFlag(metadata.Flags, Properties::PropertyFlags::AffectsRender)) {
        InvalidateVisual();
    }
}

}
