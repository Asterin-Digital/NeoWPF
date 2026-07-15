#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

NeoWPF::Rendering::ClipRect FrameworkElement::GetClipRect() const noexcept {
    NeoWPF::Rendering::ClipRect clip{};
    if (m_clipToBounds && m_finalWidth > 0 && m_finalHeight > 0) {
        int absX = 0;
        int absY = 0;
        GetAbsolutePosition(absX, absY);

        clip.x = static_cast<float>(absX);
        clip.y = static_cast<float>(absY);
        clip.w = static_cast<float>(m_finalWidth);
        clip.h = static_cast<float>(m_finalHeight);
        clip.active = true;
    }

    auto parent = dynamic_cast<FrameworkElement*>(GetParent());
    if (parent) {
        const auto parentClip = parent->GetClipRect();
        if (parentClip.active) {
            if (!clip.active) {
                return parentClip;
            }

            const float left = (std::max)(clip.x, parentClip.x);
            const float top = (std::max)(clip.y, parentClip.y);
            const float right = (std::min)(clip.x + clip.w, parentClip.x + parentClip.w);
            const float bottom = (std::min)(clip.y + clip.h, parentClip.y + parentClip.h);
            if (right <= left || bottom <= top) {
                return NeoWPF::Rendering::ClipRect{};
            }

            return NeoWPF::Rendering::ClipRect{left, top, right - left, bottom - top, true};
        }
    }

    return clip;
}

void FrameworkElement::GetAbsolutePosition(int& outX, int& outY) const noexcept
{
    float x = static_cast<float>(m_offsetX + m_renderOffsetX);
    float y = static_cast<float>(m_offsetY + m_renderOffsetY);

    auto parent = GetParent();
    while (parent)
    {
        auto fw = dynamic_cast<FrameworkElement*>(parent);
        if (fw)
        {
            float width = static_cast<float>((std::max)(fw->GetFinalWidth(), fw->GetDesiredWidth()));
            float height = static_cast<float>((std::max)(fw->GetFinalHeight(), fw->GetDesiredHeight()));

            x += static_cast<float>(fw->GetLayoutOffsetX() + fw->GetRenderOffsetX());
            y += static_cast<float>(fw->GetLayoutOffsetY() + fw->GetRenderOffsetY());
            fw->ApplyRenderTransform(x, y, width, height);
        }
        parent = parent->GetParent();
    }

    outX = static_cast<int>(std::lround(x));
    outY = static_cast<int>(std::lround(y));
}

void FrameworkElement::GetAbsoluteLayoutPosition(int& outX, int& outY) const noexcept
{
    outX = m_offsetX;
    outY = m_offsetY;

    auto parent = GetParent();
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

void FrameworkElement::ApplyRenderTransform(float& x, float& y, float& width, float& height) const noexcept
{
    auto transform = GetRenderTransform();
    if (!transform) {
        return;
    }

    const auto origin = GetRenderTransformOrigin();
    const float originX = width * origin.X;
    const float originY = height * origin.Y;

    const NeoWPF::UI::Core::Matrix4 matrix = transform->GetMatrix();
    const float translateX = matrix.m[3][0];
    const float translateY = matrix.m[3][1];

    const float absoluteOriginX = x + originX;
    const float absoluteOriginY = y + originY;

    const auto applyPoint = [&](float px, float py, float& outX, float& outY) {
        const float dx = px - absoluteOriginX;
        const float dy = py - absoluteOriginY;
        outX = matrix.m[0][0] * dx + matrix.m[0][1] * dy + absoluteOriginX + translateX;
        outY = matrix.m[1][0] * dx + matrix.m[1][1] * dy + absoluteOriginY + translateY;
    };

    float transformedOriginX = absoluteOriginX;
    float transformedOriginY = absoluteOriginY;
    applyPoint(absoluteOriginX, absoluteOriginY, transformedOriginX, transformedOriginY);

    x = transformedOriginX - originX;
    y = transformedOriginY - originY;
}

}
