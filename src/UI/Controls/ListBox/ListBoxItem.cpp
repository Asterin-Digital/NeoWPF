#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <algorithm>

namespace NeoWPF::UI {

ListBoxItem::ListBoxItem()
{
}

ListBoxItem::~ListBoxItem() = default;

void ListBoxItem::SetOwner(ListBox* owner) noexcept
{
    m_owner = owner;
}

ListBox* ListBoxItem::GetOwner() const noexcept
{
    return m_owner;
}

void ListBoxItem::SetIsSelected(bool selected) noexcept
{
    if (IsApplyingPropertyValue()) {
        InvalidateVisual();
        return;
    }

    SetValue(IsSelectedProperty(), selected);
}

bool ListBoxItem::GetIsSelected() const noexcept
{
    try {
        return GetValue<bool>(IsSelectedProperty());
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

void ListBoxItem::OnMouseUp(int x, int y, std::uint32_t buttonState)
{
    Content::ContentControl::OnMouseUp(x, y, buttonState);
}

void ListBoxItem::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (GetIsSelected()) {
        int absoluteX = 0;
        int absoluteY = 0;
        GetAbsolutePosition(absoluteX, absoluteY);
        const float width = static_cast<float>(std::max(0, GetFinalWidth()));
        const float height = static_cast<float>(std::max(0, GetFinalHeight()));

        if (width > 0.0f && height > 0.0f) {
            commands.DrawRoundedRect(
                static_cast<float>(absoluteX),
                static_cast<float>(absoluteY),
                width,
                height,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                GetBorderColor().ToArgb(),
                GetOpacity(),
                GetClipRect(),
                GetWorldRotationRadians());
            commands.DrawRoundedRect(
                static_cast<float>(absoluteX + 1),
                static_cast<float>(absoluteY + 1),
                std::max(0.0f, width - 2.0f),
                std::max(0.0f, height - 2.0f),
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                GetBackgroundColor().ToArgb(),
                GetOpacity(),
                GetClipRect(),
                GetWorldRotationRadians());
        }
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}
