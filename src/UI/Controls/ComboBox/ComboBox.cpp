#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/UI/Controls/ComboBoxItemStyles.h>
#include <NeoWPF/UI/Controls/ComboBoxStyles.h>
#include <NeoWPF/ControlStyles.h>
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

}

ComboBoxItem::ComboBoxItem()
{
    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::ComboBoxItemStyles, *this);
    SetTemplate(std::make_shared<ComboBoxItemTemplate>());
}

ComboBoxItem::~ComboBoxItem() = default;

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
    if (!manager) {
        return;
    }

    manager->GoToState(this, "CommonStates", GetIsHighlighted() ? "MouseOver" : "Normal", useTransitions);
    manager->GoToState(this, "SelectionStates", GetIsSelected() ? "Selected" : "Unselected", useTransitions);
    manager->GoToState(this, "HighlightStates", GetIsHighlighted() ? "Highlighted" : "NotHighlighted", useTransitions);
}

ComboBox::ComboBox()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug, "[ComboBox] Constructing comboBox=%p", this);
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ComboBoxTemplate>());
    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::ComboBoxStyles, *this);
    SetPadding(8);

    m_display = std::make_shared<Content::ContentPresenter>();
    m_display->SetPadding(8);
    SetPresenter(m_display);
    m_display->SetContentData(std::any(std::wstring(L"Select an item")));

    m_popup = std::make_shared<Popup>();
    m_popup->SetPlacementTarget(this);
    m_popup->SetPlacement(GetPlacement());
    m_popup->SetStaysOpen(false);

    m_dropDown = std::make_shared<ComboBoxDropDown>(this);
    m_dropDown->SetPadding(4);
    m_popup->SetContent(m_dropDown);

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
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ComboBox,
        NeoWPF::Diagnostics::Level::Debug, "[ComboBox] Destroyed comboBox=%p", this);
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
    m_popup->SetPlacement(placement);
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
        "[ComboBox] BuildRenderCommands comboBox=%p open=%d final=%dx%d popupOpen=%d",
        this, GetIsDropDownOpen() ? 1 : 0, GetFinalWidth(), GetFinalHeight(),
        m_popup && m_popup->GetIsOpen() ? 1 : 0);
    int absoluteX = 0;
    int absoluteY = 0;
    GetAbsolutePosition(absoluteX, absoluteY);

    const float x = static_cast<float>(absoluteX);
    const float y = static_cast<float>(absoluteY);
    const float width = static_cast<float>(std::max(0, GetFinalWidth()));
    const float height = static_cast<float>(std::max(0, GetFinalHeight()));

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(absoluteX, absoluteY, static_cast<int>(width), static_cast<int>(height));
        background->SetOpacity(GetOpacity());
        background->BuildRenderCommands(commands);
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const auto padding = GetPadding();
        contentHost->SetRect(
            absoluteX + padding.Left,
            absoluteY + padding.Top,
            std::max(0, static_cast<int>(width) - padding.Horizontal()),
            std::max(0, static_cast<int>(height) - padding.Vertical()));
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}
