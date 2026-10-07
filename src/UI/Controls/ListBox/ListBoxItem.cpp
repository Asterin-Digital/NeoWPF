#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <algorithm>

namespace NeoWPF::UI {

ListBoxItem::ListBoxItem()
{
    SetPresenter(std::make_shared<NeoWPF::UI::Content::ContentPresenter>());
    ApplyTheme();
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyTheme();
    });
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBoxItem] Constructed item=%p presenter=%p", this, GetPresenter().get());
}

ListBoxItem::~ListBoxItem()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
}

void ListBoxItem::ApplyTheme()
{
    using NeoWPF::Core::ApplicationThemes::PaletteColor;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;

    auto& themeManager = ThemeManager::Instance();
    SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Surface));
    SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Border));

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::ListBoxItem, padding)) {
        SetThemePropertyValue("Padding", padding);
    }
}

void ListBoxItem::SetOwner(ListBox* owner) noexcept
{
    m_owner = owner;
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBoxItem] SetOwner item=%p owner=%p parent=%p", this, owner, GetParent());
}

ListBox* ListBoxItem::GetOwner() const noexcept
{
    return m_owner;
}

void ListBoxItem::SetIsSelected(bool selected) noexcept
{
    if (IsApplyingPropertyValue()) {
        InvalidateVisual();
        OnSelectionStateChanged();
        return;
    }

    SetValue(IsSelectedProperty(), selected);
    OnSelectionStateChanged();
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
    const auto before = commands.commands.size();
    const auto content = GetContentElement();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBoxItem] BuildRenderCommands item=%p parent=%p presenter=%p presenterParent=%p content=%p contentParent=%p bounds=%dx%d selected=%d commandsBefore=%zu",
        this, GetParent(), GetPresenter().get(), GetPresenter() ? GetPresenter()->GetParent() : nullptr,
        content.get(), content ? content->GetParent() : nullptr,
        GetFinalWidth(), GetFinalHeight(), GetIsSelected(), before);
    if (GetIsSelected()) {
            const auto renderTransform = GetWorldRenderTransform();
            double absoluteX = 0.0;
            double absoluteY = 0.0;
            renderTransform.positionMatrix.TransformPoint(0.0f, 0.0f, absoluteX, absoluteY);
        const double width = (std::max)(0.0, GetFinalWidth());
        const double height = (std::max)(0.0, GetFinalHeight());

        if (width > 0.0f && height > 0.0f) {
                commands.DrawRoundedRect(
                    absoluteX,
                    absoluteY,
                    width,
                    height,
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    GetBorderColor().ToArgb(),
                    GetOpacity(),
                    GetClipRect(),
                    renderTransform.rotationRadians,
                    renderTransform.pivot.X,
                    renderTransform.pivot.Y);
                commands.DrawRoundedRect(
                    absoluteX + 1.0,
                    absoluteY + 1.0,
                    std::max(0.0, width - 2.0),
                    std::max(0.0, height - 2.0),
                    0.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    GetBackgroundColor().ToArgb(),
                    GetOpacity(),
                    GetClipRect(),
                    renderTransform.rotationRadians,
                    renderTransform.pivot.X,
                    renderTransform.pivot.Y);
        }
    }

    Content::ContentControl::BuildRenderCommands(commands);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ListBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ListBoxItem] BuildRenderCommands complete item=%p commandsAfter=%zu added=%zu",
        this, commands.commands.size(), commands.commands.size() - before);
}

}
