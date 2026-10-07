#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {
namespace {

class ComboBoxDropDown final : public ListBox {
public:
    explicit ComboBoxDropDown(ComboBox* owner)
        : m_owner(owner)
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Debug,
            "[ComboBox] DropDown constructed dropDown=%p owner=%p", this, owner);
        SetItemTemplate([](const std::any&, int) {
            return std::shared_ptr<FrameworkElement>{};
        });
    }

    void SyncSelection(const std::any& item) override
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Trace,
            "[ComboBox] DropDown sync selection dropDown=%p hasItem=%d type=%s",
            this, item.has_value() ? 1 : 0, item.has_value() ? item.type().name() : "none");
        m_syncingFromOwner = true;
        SetSelectedItem(item);
        m_syncingFromOwner = false;
    }

    std::shared_ptr<FrameworkElement> GetContainerForItem(const std::any& data, int index) override
    {
        (void)index;
        auto item = std::make_shared<ComboBoxItem>();
        item->SetDataContext(data);
        return item;
    }

    void PrepareContainer(const std::shared_ptr<FrameworkElement>& container,
                          const std::any& data,
                          int index) override
    {
        ListBox::PrepareContainer(container, data, index);
    }

    void ApplyItemTemplate(const std::shared_ptr<FrameworkElement>& container,
                           const std::any& data,
                           int index,
                           const Controls::ItemsControl::Virtualization::ItemTemplate& templ) override
    {
        (void)index;
        auto item = std::dynamic_pointer_cast<ComboBoxItem>(container);
        if (!item) {
            return;
        }

        if (templ) {
            if (auto content = templ(data, index)) {
                item->SetContent(content);
                return;
            }
        }
        item->SetContentData(data);
    }

    void OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem) override
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Debug,
            "[ComboBox] DropDown selection changed dropDown=%p oldIndex=%d newIndex=%d hasNewItem=%d syncing=%d",
            this, oldIndex, newIndex, newItem.has_value() ? 1 : 0, m_syncingFromOwner ? 1 : 0);
        ListBox::OnSelectionChanged(oldIndex, newIndex, oldItem, newItem);
        if (m_owner && !m_syncingFromOwner && newItem.has_value()) {
            m_owner->SetSelectedItem(newItem);
            m_owner->SetIsDropDownOpen(false);
        }
    }

private:
    ComboBox* m_owner = nullptr;
    bool m_syncingFromOwner = false;
};

void ApplyComboBoxTheme(ComboBox& comboBox)
{
    using NeoWPF::Core::ApplicationThemes::PaletteColor;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;

    comboBox.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Surface));
    comboBox.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Border));

    NeoWPF::UI::Thickness padding;
    if (ThemeManager::Instance().TryGetPadding(ThemePadding::ComboBox, padding)) {
        comboBox.SetThemePropertyValue("Padding", padding);
    }
}

void ApplyComboBoxItemTheme(ComboBoxItem& item)
{
    using NeoWPF::Core::ApplicationThemes::PaletteColor;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;

    auto& themeManager = ThemeManager::Instance();
    item.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Surface));
    item.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteColor(PaletteColor::Border));
    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::ComboBoxItem, padding)) {
        item.SetThemePropertyValue("Padding", padding);
    }
}

}

ComboBoxItem::ComboBoxItem()
{
    SetTemplate(std::make_shared<ComboBoxItemTemplate>());
    ApplyTheme();
}

ComboBoxItem::~ComboBoxItem() = default;

void ComboBoxItem::ApplyTheme()
{
    ApplyComboBoxItemTheme(*this);
    ApplyVisualStates(false);
}

void ComboBoxItem::SetIsHighlighted(bool highlighted) noexcept
{
    if (IsApplyingPropertyValue()) {
        ApplyVisualStates(false);
        return;
    }

    SetValue(IsHighlightedProperty(), highlighted);
    ApplyVisualStates(true);
}

bool ComboBoxItem::GetIsHighlighted() const noexcept
{
    try {
        return GetValue<bool>(IsHighlightedProperty());
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

void ComboBoxItem::OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args)
{
    (void)args;
    SetIsHighlighted(true);
}

void ComboBoxItem::OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args)
{
    (void)args;
    SetIsHighlighted(false);
}

void ComboBoxItem::OnSelectionStateChanged()
{
    ApplyVisualStates(true);
}

void ComboBoxItem::ApplyVisualStates(bool useTransitions)
{
    auto manager = GetTemplateVisualStateManager();
    if (manager) {
        manager->GoToState(this, "CommonStates", GetIsHighlighted() ? "MouseOver" : "Normal", useTransitions);
        manager->GoToState(this, "SelectionStates", GetIsSelected() ? "Selected" : "Unselected", useTransitions);
        manager->GoToState(this, "HighlightStates", GetIsHighlighted() ? "Highlighted" : "NotHighlighted", useTransitions);
    }

    if (auto part = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        part->SetFillColor(GetIsHighlighted()
            ? NeoWPF::Core::ApplicationThemes::GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::AccentHover).ToArgb()
            : NeoWPF::UI::Colors::Transparent.ToArgb());
        part->SetBorderColor(GetIsHighlighted()
            ? NeoWPF::Core::ApplicationThemes::GetPaletteColor(NeoWPF::Core::ApplicationThemes::PaletteColor::Focus).ToArgb()
            : NeoWPF::UI::Colors::Transparent.ToArgb());
        part->SetBorderThickness(GetIsHighlighted() ? 1 : 0);
    }
    if (auto part = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_SelectionOverlay"))) {
        part->SetFillColor(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
            NeoWPF::Core::ApplicationThemes::PaletteColor::AccentPressed).ToArgb());
    }
    if (auto part = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_HighlightOverlay"))) {
        part->SetFillColor(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
            NeoWPF::Core::ApplicationThemes::PaletteColor::AccentHover).ToArgb());
    }
    InvalidateVisual();
}

ComboBox::ComboBox()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug, "[ComboBox] Constructing comboBox=%p", this);
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ComboBoxTemplate>());

    m_display = std::make_shared<Content::ContentPresenter>();
    SetPresenter(m_display);
    m_display->SetContentData(std::any(std::wstring(L"Select an item")));

    m_dropDown = std::make_shared<ComboBoxDropDown>(this);
    ApplyComboBoxTheme(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyComboBoxTheme(*this);
    });

    AddHandler(NeoWPF::Input::Core::MouseUpEvent(), [this](UIElement*, NeoWPF::Input::Core::RoutedEventArgs* args) {
        const bool wasOpen = GetIsDropDownOpen();
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Info,
            "[ComboBox] MouseUp toggle comboBox=%p wasOpen=%d", this, wasOpen ? 1 : 0);
        SetIsDropDownOpen(!wasOpen);
        if (args) {
            args->SetHandled(true);
        }
    });

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] Constructed comboBox=%p popup=%p dropDown=%p", this, m_popup.get(), m_dropDown.get());
}

ComboBox::~ComboBox()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug, "[ComboBox] Destroyed comboBox=%p", this);
}

void ComboBox::OnPaddingChanged(Thickness padding)
{
    if (m_display) {
        m_display->SetPadding(padding);
    }
    InvalidateMeasure();
    InvalidateVisual();
}

void ComboBox::EnsurePopupTemplateParts()
{
    if (!m_popup) {
        m_popup = std::dynamic_pointer_cast<Popup>(FindTemplateChild("PART_Popup"));
    }
    if (!m_popup) {
        return;
    }

    m_popup->SetPlacementTarget(this);
    m_popup->SetPlacement(GetPlacement());
    m_popup->SetStaysOpen(false);

    if (auto popupBorder = std::dynamic_pointer_cast<Border>(FindTemplateChild("PART_PopupBorder"))) {
        popupBorder->SetChild(m_dropDown);
        m_popup->SetContent(popupBorder);
    } else {
        m_popup->SetContent(m_dropDown);
    }
}

void ComboBox::OnBackgroundColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetFillColor(color.ToArgb());
    }
    InvalidateVisual();
}

void ComboBox::OnBorderColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderColor(color.ToArgb());
    }
    InvalidateVisual();
}

bool ComboBox::GetIsDropDownOpen() const noexcept
{
    try {
        return GetValue<bool>(IsDropDownOpenProperty());
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

void ComboBox::SetIsDropDownOpen(bool isOpen)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Info,
        "[ComboBox] SetIsDropDownOpen comboBox=%p requested=%d current=%d applying=%d popupOpen=%d",
        this, isOpen ? 1 : 0, GetIsDropDownOpen() ? 1 : 0,
        IsApplyingPropertyValue() ? 1 : 0, m_popup && m_popup->GetIsOpen() ? 1 : 0);
    if (!IsApplyingPropertyValue()) {
        SetValue(IsDropDownOpenProperty(), isOpen);
        return;
    }
    EnsurePopupTemplateParts();
    if (!m_popup) {
        return;
    }
    m_popup->SetIsOpen(isOpen);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] Popup state applied comboBox=%p popupOpen=%d", this, m_popup->GetIsOpen() ? 1 : 0);
}

PopupPlacement ComboBox::GetPlacement() const noexcept
{
    try {
        return GetValue<PopupPlacement>(PlacementProperty());
    } catch (const std::bad_any_cast&) {
        return PopupPlacement::Bottom;
    }
}

void ComboBox::SetPlacement(PopupPlacement placement)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] SetPlacement comboBox=%p placement=%d applying=%d",
        this, static_cast<int>(placement), IsApplyingPropertyValue() ? 1 : 0);
    if (!IsApplyingPropertyValue()) {
        SetValue(PlacementProperty(), placement);
        return;
    }
    if (m_popup) {
        m_popup->SetPlacement(placement);
    }
}

void ComboBox::SetItems(const std::vector<std::shared_ptr<FrameworkElement>>& items)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] SetItems comboBox=%p count=%zu", this, items.size());
    SetSelectedItem(std::any{});
    Selector::ClearItems();
    m_dropDown->SetItems(items);
}

void ComboBox::AddItem(std::shared_ptr<FrameworkElement> item)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ComboBox] AddItem comboBox=%p item=%p", this, item.get());
    m_dropDown->AddItem(std::move(item));
}

void ComboBox::ClearItems()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug, "[ComboBox] ClearItems comboBox=%p", this);
    SetSelectedItem(std::any{});
    Selector::ClearItems();
    m_dropDown->ClearItems();
}

void ComboBox::SetItemsSource(const std::vector<std::any>& source)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] SetItemsSource comboBox=%p count=%zu", this, source.size());
    SetSelectedItem(std::any{});
    Selector::SetItemsSource(source);
    m_dropDown->SetItemsSource(source);
}

void ComboBox::SetItemTemplate(const Controls::ItemsControl::Virtualization::ItemTemplate& templ)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ComboBox] SetItemTemplate comboBox=%p hasTemplate=%d", this, templ ? 1 : 0);
    Selector::SetItemTemplate(templ);
    m_dropDown->SetItemTemplate(templ);
}

void ComboBox::OnSelectionChanged(int oldIndex, int newIndex, const std::any& oldItem, const std::any& newItem)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug,
        "[ComboBox] Selection changed comboBox=%p oldIndex=%d newIndex=%d hasNewItem=%d",
        this, oldIndex, newIndex, newItem.has_value() ? 1 : 0);
    (void)oldIndex;
    (void)newIndex;
    (void)oldItem;
    SetValue(SelectionBoxItemProperty(), newItem);
    if (m_dropDown) {
        m_dropDown->SyncSelection(newItem);
    }
    UpdateDisplayedItem(newItem);
}

std::any ComboBox::GetSelectionBoxItem() const noexcept
{
    try {
        return GetValue<std::any>(SelectionBoxItemProperty());
    } catch (const std::bad_any_cast&) {
        return {};
    }
}

void ComboBox::UpdateDisplayedItem(const std::any& item)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ComboBox] UpdateDisplayedItem comboBox=%p hasItem=%d type=%s",
        this, item.has_value() ? 1 : 0, item.has_value() ? item.type().name() : "none");
    if (m_display) {
        m_display->SetContentData(item);
    }
}

void ComboBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Trace,
        "[ComboBox] BuildRenderCommands comboBox=%p open=%d final=%.1fx%.1f popupOpen=%d",
        this, GetIsDropDownOpen() ? 1 : 0, GetFinalWidth(), GetFinalHeight(),
        m_popup && m_popup->GetIsOpen() ? 1 : 0);
    const auto renderTransform = GetWorldRenderTransform();
    double absoluteX = 0.0;
    double absoluteY = 0.0;
    renderTransform.positionMatrix.TransformPoint(0.0f, 0.0f, absoluteX, absoluteY);

    const double x = absoluteX;
    const double y = absoluteY;
    const double width = std::max(0.0, GetFinalWidth());
    const double height = std::max(0.0, GetFinalHeight());

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        const auto commandsBefore = commands.commands.size();
        background->SetFillColor(GetBackgroundColor().ToArgb());
        background->SetBorderColor(GetBorderColor().ToArgb());
        background->SetRect(x, y, width, height);
        background->SetOpacity(GetOpacity());
        background->BuildRenderCommands(commands);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Debug,
            "[ComboBox] Background emitted comboBox=%p commands=%zu->%zu rect=%.1fx%.1f opacity=%.2f",
            this, commandsBefore, commands.commands.size(), width, height, GetOpacity());
    } else {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
            NeoWPF::Diagnostics::Level::Info,
            "[ComboBox] PART_Background missing comboBox=%p", this);
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const auto padding = GetPadding();
        contentHost->SetRect(
            x + padding.Left,
            y + padding.Top,
            std::max(0.0, width - padding.Horizontal()),
            std::max(0.0, height - padding.Vertical()));
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}
