#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Markup/Xaml/XamlPropertyBinding.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <unordered_map>

namespace NeoWPF::UI::Content {



ContentControl::ContentControl()
    : m_content(nullptr)
    , m_presenter(nullptr)
{
    ContentProperty();
}

const NeoWPF::UI::Properties::DependencyProperty& ContentControl::ContentProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        ContentControl, std::shared_ptr<NeoWPF::UI::UIElement>>(
            "Content");
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlElementProperty<
        ContentControl, NeoWPF::UI::UIElement>(property, &ContentControl::SetContent), true);
    (void)bindingRegistered;
    return property;
}

void ContentControl::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
        "[ContentControl] SetContent: content=%p presenter=%p", content.get(), m_presenter.get());

    m_content = content;
    if (m_presenter) {
        m_presenter->SetContent(content);
    }
    InvalidateVisual();
}

void ContentControl::ClearContent()
{
    m_content.reset();
    if (m_presenter) {
        m_presenter->ClearContent();
    }
    InvalidateVisual();
}

void ContentControl::SetPresenter(const std::shared_ptr<ContentPresenter>& presenter)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
        "[ContentControl] SetPresenter: oldPresenter=%p newPresenter=%p content=%p", m_presenter.get(), presenter.get(), m_content.get());

    if (m_presenter == presenter) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] SetPresenter: same presenter, skipping");
        return;
    }

    static std::unordered_map<const void*, ContentControl*> s_presenterOwners;

    if (m_presenter) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] SetPresenter: removing old presenter %p", m_presenter.get());
        s_presenterOwners.erase(static_cast<const void*>(m_presenter.get()));
        m_presenter->RemoveFromParent();
    }

    std::shared_ptr<ContentPresenter> toAssign = presenter;
    if (presenter) {
        auto existingParent = presenter->GetParent();
        auto it = s_presenterOwners.find(static_cast<const void*>(presenter.get()));
        ContentControl* recordedOwner = (it != s_presenterOwners.end()) ? it->second : nullptr;

        if (existingParent || recordedOwner) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
                "[ContentControl] SetPresenter: presenter %p already has parent=%p recordedOwner=%p; creating fresh presenter", presenter.get(), existingParent, recordedOwner);
            toAssign = std::make_shared<ContentPresenter>();
        }
    }

    m_presenter = toAssign;
    if (m_presenter) {
        s_presenterOwners[static_cast<const void*>(m_presenter.get())] = this;
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] SetPresenter: using new presenter %p", m_presenter.get());
        m_presenter->SetContent(m_content);
    }
    InvalidateVisual();
}

void ContentControl::Measure(int availableWidth, int availableHeight)
{
if (m_presenter && !m_presenter->GetParent()) {
    try {
        AddChild(m_presenter);
    } catch (const std::bad_weak_ptr&) {
        NeoWPF::Diagnostics::Log(
            NeoWPF::Diagnostics::Category::Control,
            NeoWPF::Diagnostics::Level::Info,
            "[ContentControl] Measure: failed to AddChild presenter=%p - bad_weak_ptr (parent may have been destroyed)",
            m_presenter.get());
    }
}

    if (m_presenter) {
        m_presenter->Measure(availableWidth, availableHeight);

        const int presenterWidth = m_presenter->GetDesiredWidth();
        const int presenterHeight = m_presenter->GetDesiredHeight();
        const int explicitWidth = GetWidth() > 0 ? GetWidth() : 0;
        const int explicitHeight = GetHeight() > 0 ? GetHeight() : 0;

        SetDesiredSize(
            std::max(presenterWidth, explicitWidth),
            std::max(presenterHeight, explicitHeight));
        return;
    }

    FrameworkElement::Measure(availableWidth, availableHeight);
    SetDesiredSize(0, 0);
}

void ContentControl::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
        "[ContentControl] Arrange: this=%p final=(%d,%d) presenter=%p", this, finalWidth, finalHeight, m_presenter.get());
    if (m_presenter) {
        m_presenter->Arrange(GetFinalWidth(), GetFinalHeight());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] Arrange: presenter arranged=%p final=(%d,%d)", m_presenter.get(), m_presenter->GetFinalWidth(), m_presenter->GetFinalHeight());
    }
}

void ContentControl::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_presenter) {
        m_presenter->BuildRenderCommands(commands);
    }
}

ContentBounds ContentControl::GetContentBounds() const noexcept
{
    if (m_presenter) {
        return ContentBounds{m_presenter->GetFinalWidth(), m_presenter->GetFinalHeight()};
    }

    return ContentBounds{};
}

}
