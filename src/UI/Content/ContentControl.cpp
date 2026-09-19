#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <unordered_map>

namespace NeoWPF::UI::Content {

namespace {

std::unordered_map<const void*, ContentControl*> s_presenterOwners;

}


ContentControl::ContentControl()
    : m_content(nullptr)
    , m_presenter(nullptr)
{
    ContentProperty();
}

ContentControl::~ContentControl()
{
    if (m_presenter) {
        s_presenterOwners.erase(static_cast<const void*>(m_presenter.get()));
    }
}

const NeoWPF::UI::Properties::DependencyProperty& ContentControl::ContentProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        ContentControl, std::any>("Content", [] {
            NeoWPF::UI::Properties::PropertyMetadata metadata;
            metadata.DefaultValue = std::any{};
            metadata.Converter = NeoWPF::UI::Properties::GetContentConverter();
            metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure |
                NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
            metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender,
                                                  const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
                if (auto owner = dynamic_cast<ContentControl*>(sender)) {
                    std::any convertedValue;
                    if (NeoWPF::UI::Properties::TryConvertValue(
                            ContentProperty(), args.NewValue, convertedValue)) {
                        owner->SetContentData(convertedValue);
                    } else {
                        owner->SetContentData(args.NewValue);
                    }
                }
            };
            return metadata;
        }());
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlElementPropertyCallback<
        ContentControl, NeoWPF::UI::UIElement>(property,
            [](ContentControl& owner, const std::shared_ptr<NeoWPF::UI::UIElement>& content) {
                owner.SetContent(content);
            }), true);
    (void)bindingRegistered;
    return property;
}

void ContentControl::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    SetContentData(std::any(content));
}

void ContentControl::SetContentData(const std::any& content)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
        "[ContentControl] SetContent: contentType=%s presenter=%p",
        content.has_value() ? content.type().name() : "null", m_presenter.get());

    if (!IsApplyingPropertyValue()) {
        SetValue(ContentProperty(), content);
        return;
    }

    m_content = content;
    if (m_presenter) {
        m_presenter->SetContentData(content);
    }
    InvalidateVisual();
}

std::shared_ptr<NeoWPF::UI::UIElement> ContentControl::GetContentElement() const noexcept
{
    if (!m_content.has_value()) {
        return nullptr;
    }

    try {
        return std::any_cast<std::shared_ptr<NeoWPF::UI::UIElement>>(m_content);
    } catch (const std::bad_any_cast&) {
        return nullptr;
    }
}

void ContentControl::ClearContent()
{
    SetContentData(std::any{});
    if (m_presenter) {
        m_presenter->ClearContent();
    }
    InvalidateVisual();
}

void ContentControl::SetPresenter(const std::shared_ptr<ContentPresenter>& presenter)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
        "[ContentControl] SetPresenter: oldPresenter=%p newPresenter=%p contentType=%s",
        m_presenter.get(), presenter.get(), m_content.has_value() ? m_content.type().name() : "null");

    if (m_presenter == presenter) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] SetPresenter: same presenter, skipping");
        return;
    }

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
        if (!m_presenter->GetParent()) {
            try {
                AddChild(m_presenter);
            } catch (const std::bad_weak_ptr&) {
                NeoWPF::Diagnostics::Log(
                    NeoWPF::Diagnostics::Category::Control,
                    NeoWPF::Diagnostics::Level::Info,
                    "[ContentControl] SetPresenter: failed to AddChild presenter=%p - bad_weak_ptr",
                    m_presenter.get());
            }
        }
        m_presenter->SetContentData(m_content);
    }
    InvalidateVisual();
}

void ContentControl::Measure(double availableWidth, double availableHeight)
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

        const double presenterWidth = m_presenter->GetDesiredWidth();
        const double presenterHeight = m_presenter->GetDesiredHeight();
        const double explicitWidth = GetWidth() > 0.0 ? GetWidth() : 0.0;
        const double explicitHeight = GetHeight() > 0.0 ? GetHeight() : 0.0;

        SetDesiredSize(
            std::max(presenterWidth, explicitWidth),
            std::max(presenterHeight, explicitHeight));
        return;
    }

    FrameworkElement::Measure(availableWidth, availableHeight);
    SetDesiredSize(0, 0);
}

void ContentControl::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
        "[ContentControl] Arrange: this=%p final=(%g,%g) presenter=%p", this, finalWidth, finalHeight, m_presenter.get());
    if (m_presenter) {
        m_presenter->Arrange(GetFinalWidth(), GetFinalHeight());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
            "[ContentControl] Arrange: presenter arranged=%p final=(%g,%g)", m_presenter.get(), m_presenter->GetFinalWidth(), m_presenter->GetFinalHeight());
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
