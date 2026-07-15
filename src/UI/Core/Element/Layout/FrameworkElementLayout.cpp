#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <algorithm>
#include <typeinfo>

namespace NeoWPF::UI {

bool FrameworkElement::s_frameworkElementLogging = false;

LayoutSizeMode FrameworkElement::GetWidthMode() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("WidthMode", typeid(*this));
    if (property) {
        try {
            return GetValue<LayoutSizeMode>(*property);
        } catch (const std::bad_any_cast&) {
            return m_widthMode;
        }
    }

    return m_widthMode;
}

LayoutSizeMode FrameworkElement::GetHeightMode() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HeightMode", typeid(*this));
    if (property) {
        try {
            return GetValue<LayoutSizeMode>(*property);
        } catch (const std::bad_any_cast&) {
            return m_heightMode;
        }
    }

    return m_heightMode;
}

int FrameworkElement::GetWidth() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Width", typeid(*this));
    if (property) {
        try {
            return GetValue<int>(*property);
        } catch (const std::bad_any_cast&) {
            return m_width;
        }
    }

    return m_width;
}

void FrameworkElement::SetWidth(int width) noexcept
{
    const int clampedWidth = width < 0 ? 0 : width;
    if (IsApplyingPropertyValue()) {
        m_width = clampedWidth;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Width", typeid(*this));
    if (property) {
        SetValue(*property, clampedWidth);
        return;
    }

    m_width = clampedWidth;
    InvalidateVisual();
}

int FrameworkElement::GetHeight() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Height", typeid(*this));
    if (property) {
        try {
            return GetValue<int>(*property);
        } catch (const std::bad_any_cast&) {
            return m_height;
        }
    }

    return m_height;
}

void FrameworkElement::SetHeight(int height) noexcept
{
    const int clampedHeight = height < 0 ? 0 : height;
    if (IsApplyingPropertyValue()) {
        m_height = clampedHeight;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Height", typeid(*this));
    if (property) {
        SetValue(*property, clampedHeight);
        return;
    }

    m_height = clampedHeight;
    InvalidateVisual();
}

bool FrameworkElement::IsClipToBounds() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("ClipToBounds", typeid(*this));
    if (property) {
        try {
            return GetValue<bool>(*property);
        } catch (const std::bad_any_cast&) {
            return m_clipToBounds;
        }
    }

    return m_clipToBounds;
}

void FrameworkElement::SetClipToBounds(bool clipToBounds) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_clipToBounds = clipToBounds;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("ClipToBounds", typeid(*this));
    if (property) {
        SetValue(*property, clipToBounds);
        return;
    }

    if (m_clipToBounds != clipToBounds) {
        m_clipToBounds = clipToBounds;
        InvalidateVisual();
    }
}

void FrameworkElement::SetWidthMode(LayoutSizeMode mode) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_widthMode = mode;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("WidthMode", typeid(*this));
    if (property) {
        SetValue(*property, mode);
        return;
    }

    m_widthMode = mode;
    InvalidateVisual();
}

void FrameworkElement::SetHeightMode(LayoutSizeMode mode) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_heightMode = mode;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HeightMode", typeid(*this));
    if (property) {
        SetValue(*property, mode);
        return;
    }

    m_heightMode = mode;
    InvalidateVisual();
}

NeoWPF::UI::CornerRadius FrameworkElement::GetCornerRadius() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CornerRadius", typeid(*this));
    if (property) {
        try {
            return GetValue<NeoWPF::UI::CornerRadius>(*property);
        } catch (const std::bad_any_cast&) {
            return m_cornerRadius;
        }
    }

    return m_cornerRadius;
}

void FrameworkElement::SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_cornerRadius = cornerRadius;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CornerRadius", typeid(*this));
    if (property) {
        SetValue(*property, cornerRadius);
        return;
    }

    if (m_cornerRadius != cornerRadius) {
        m_cornerRadius = cornerRadius;
        InvalidateVisual();
    }
}

HorizontalAlignment FrameworkElement::GetHorizontalAlignment() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalAlignment", typeid(*this));
    if (property) {
        try {
            return GetValue<HorizontalAlignment>(*property);
        } catch (const std::bad_any_cast&) {
            return m_horizontalAlignment;
        }
    }

    return m_horizontalAlignment;
}

void FrameworkElement::SetHorizontalAlignment(HorizontalAlignment align) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_horizontalAlignment = align;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalAlignment", typeid(*this));
    if (property) {
        SetValue(*property, align);
        return;
    }

    m_horizontalAlignment = align;
}

VerticalAlignment FrameworkElement::GetVerticalAlignment() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("VerticalAlignment", typeid(*this));
    if (property) {
        try {
            return GetValue<VerticalAlignment>(*property);
        } catch (const std::bad_any_cast&) {
            return m_verticalAlignment;
        }
    }

    return m_verticalAlignment;
}

void FrameworkElement::SetVerticalAlignment(VerticalAlignment align) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_verticalAlignment = align;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("VerticalAlignment", typeid(*this));
    if (property) {
        SetValue(*property, align);
        return;
    }

    m_verticalAlignment = align;
}

int FrameworkElement::ResolveWidth(int availableWidth, int desiredWidth) const
{
    if (GetWidthMode() == LayoutSizeMode::Stretch) {
        return (std::max)(0, availableWidth);
    }

    if (!GetParent() && availableWidth > 0) {
        return (std::max)(0, availableWidth);
    }

    const int preferredWidth = GetWidth() > 0 ? GetWidth() : desiredWidth;
    return (std::max)(0, preferredWidth);
}

int FrameworkElement::ResolveHeight(int availableHeight, int desiredHeight) const
{
    if (GetHeightMode() == LayoutSizeMode::Stretch) {
        return (std::max)(0, availableHeight);
    }

    if (!GetParent() && availableHeight > 0) {
        return (std::max)(0, availableHeight);
    }

    const int preferredHeight = GetHeight() > 0 ? GetHeight() : desiredHeight;
    return (std::max)(0, preferredHeight);
}

void FrameworkElement::BeginAnimation(int fromWidth, int toWidth, int fromHeight, int toHeight, float durationSeconds)
{
    using namespace NeoWPF::UI::Animation;
    auto storyboard = std::make_shared<Storyboard>();
    auto widthTimeline = std::make_shared<Timeline>(static_cast<float>(fromWidth), static_cast<float>(toWidth), durationSeconds, [this](float value) {
        this->SetWidth(static_cast<int>(value));
    });
    auto heightTimeline = std::make_shared<Timeline>(static_cast<float>(fromHeight), static_cast<float>(toHeight), durationSeconds, [this](float value) {
        this->SetHeight(static_cast<int>(value));
    });
    storyboard->AddTimeline(widthTimeline);
    storyboard->AddTimeline(heightTimeline);
    NeoWPF::UI::AddStoryboard(storyboard);
}

void FrameworkElement::BeginAnimation(HorizontalAlignment horizontal, VerticalAlignment vertical, float durationSeconds)
{
    auto parent = GetParent();
    if (!parent) {
        return;
    }

    auto parentFE = dynamic_cast<FrameworkElement*>(parent);
    int parentWidth = parentFE ? (std::max)(parentFE->GetFinalWidth(), parentFE->GetDesiredWidth()) : 0;
    int parentHeight = parentFE ? (std::max)(parentFE->GetFinalHeight(), parentFE->GetDesiredHeight()) : 0;

    int childWidth = (std::max)(GetFinalWidth(), GetDesiredWidth());
    int childHeight = (std::max)(GetFinalHeight(), GetDesiredHeight());

    int targetX = 0;
    switch (horizontal) {
        case HorizontalAlignment::Left: targetX = 0; break;
        case HorizontalAlignment::Center: targetX = (parentWidth - childWidth) / 2; break;
        case HorizontalAlignment::Right: targetX = parentWidth - childWidth; break;
        case HorizontalAlignment::Stretch: targetX = 0; break;
    }

    int targetY = 0;
    switch (vertical) {
        case VerticalAlignment::Top: targetY = 0; break;
        case VerticalAlignment::Center: targetY = (parentHeight - childHeight) / 2; break;
        case VerticalAlignment::Bottom: targetY = parentHeight - childHeight; break;
        case VerticalAlignment::Stretch: targetY = 0; break;
    }

    using namespace NeoWPF::UI::Animation;
    auto storyboard = std::make_shared<Storyboard>();
    auto xTimeline = std::make_shared<Timeline>(static_cast<float>(GetRenderOffsetX()), static_cast<float>(targetX - GetLayoutOffsetX()), durationSeconds, [this](float value) {
        this->SetRenderOffset(static_cast<int>(value), this->GetRenderOffsetY());
    });
    auto yTimeline = std::make_shared<Timeline>(static_cast<float>(GetRenderOffsetY()), static_cast<float>(targetY - GetLayoutOffsetY()), durationSeconds, [this](float value) {
        this->SetRenderOffset(this->GetRenderOffsetX(), static_cast<int>(value));
    });
    storyboard->AddTimeline(xTimeline);
    storyboard->AddTimeline(yTimeline);
    NeoWPF::UI::AddStoryboard(storyboard);
}

void FrameworkElement::Measure(int availableWidth, int availableHeight)
{
    if (s_frameworkElementLogging) {
        NeoWPF::Diagnostics::LogInfo(
            "[FrameworkElement::Measure] Type=%s, Available=(%d x %d)",
            typeid(*this).name(),
            availableWidth,
            availableHeight
        );
    }
    m_desiredWidth = availableWidth;
    m_desiredHeight = availableHeight;
}

void FrameworkElement::Arrange(int finalWidth, int finalHeight)
{
    if (s_frameworkElementLogging) {
        NeoWPF::Diagnostics::LogInfo(
            "[FrameworkElement::Arrange] Type=%s, Final=(%d x %d)",
            typeid(*this).name(),
            finalWidth,
            finalHeight
        );

        NeoWPF::Diagnostics::CallStackEntry stack[4];
        int count = NeoWPF::Diagnostics::CaptureCallStack(stack, 4, 1);
        for (int i = 0; i < count; ++i) {
            if (stack[i].symbol[0] != '\0') {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] %s @ %p", i, stack[i].symbol, stack[i].address);
            } else {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] <unknown> @ %p", i, stack[i].address);
            }
        }
    }

    m_finalWidth = ResolveWidth(finalWidth, m_desiredWidth);
    m_finalHeight = ResolveHeight(finalHeight, m_desiredHeight);
}

}
