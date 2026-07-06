#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <algorithm>

#ifdef max
#undef max
#endif

namespace NeoWPF {
namespace UI
{
    int FrameworkElement::ResolveWidth(int availableWidth, int desiredWidth) const
    {
        if (m_widthMode == LayoutSizeMode::Stretch) {
            return (std::max)(0, availableWidth);
        }

        if (!GetParent() && availableWidth > 0) {
            return (std::max)(0, availableWidth);
        }

        const int preferredWidth = m_width > 0 ? m_width : desiredWidth;
        return (std::max)(0, preferredWidth);
    }

    int FrameworkElement::ResolveHeight(int availableHeight, int desiredHeight) const
    {
        if (m_heightMode == LayoutSizeMode::Stretch) {
            return (std::max)(0, availableHeight);
        }

        if (!GetParent() && availableHeight > 0) {
            return (std::max)(0, availableHeight);
        }

        const int preferredHeight = m_height > 0 ? m_height : desiredHeight;
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

    void FrameworkElement::GetAbsolutePosition(int& outX, int& outY) const noexcept
    {
        outX = m_offsetX + m_renderOffsetX;
        outY = m_offsetY + m_renderOffsetY;

        Visual* parent = GetParent();
        while (parent)
        {
            auto fw = dynamic_cast<FrameworkElement*>(parent);
            if (fw)
            {
                outX += fw->GetLayoutOffsetX() + fw->GetRenderOffsetX();
                outY += fw->GetLayoutOffsetY() + fw->GetRenderOffsetY();
            }
            parent = parent->GetParent();
        }
    }

    void FrameworkElement::GetAbsoluteLayoutPosition(int& outX, int& outY) const noexcept
    {
        outX = m_offsetX;
        outY = m_offsetY;

        Visual* parent = GetParent();
        while (parent)
        {
            auto fw = dynamic_cast<FrameworkElement*>(parent);
            if (fw)
            {
                outX += fw->GetLayoutOffsetX();
                outY += fw->GetLayoutOffsetY();
            }
            parent = parent->GetParent();
        }
    }

    void FrameworkElement::BeginAnimation(HorizontalAlignment horizontal, VerticalAlignment vertical, float durationSeconds)
    {
        Visual* parent = GetParent();
        if (!parent) {
            return;
        }

        auto parentFE = dynamic_cast<FrameworkElement*>(parent);
        int parentWidth = parentFE ? parentFE->GetFinalWidth() : 0;
        int parentHeight = parentFE ? parentFE->GetFinalHeight() : 0;

        int childWidth = GetFinalWidth() > 0 ? GetFinalWidth() : GetDesiredWidth();
        int childHeight = GetFinalHeight() > 0 ? GetFinalHeight() : GetDesiredHeight();

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
}
}
