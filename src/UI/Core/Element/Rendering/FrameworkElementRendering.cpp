#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Diagnostics/Logger.h>
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

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
            "[GetClipRect] Element clip ACTIVE: (%g, %g, %g, %g)", clip.x, clip.y, clip.w, clip.h);
    }

    auto parent = dynamic_cast<FrameworkElement*>(GetParent());
    if (parent) {
        const auto parentClip = parent->GetClipRect();
        if (parentClip.active) {
            if (!clip.active) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
                    "[GetClipRect] Inheriting parent clip: (%g, %g, %g, %g)", parentClip.x, parentClip.y, parentClip.w, parentClip.h);
                return parentClip;
            }

            const float left = (std::max)(clip.x, parentClip.x);
            const float top = (std::max)(clip.y, parentClip.y);
            const float right = (std::min)(clip.x + clip.w, parentClip.x + parentClip.w);
            const float bottom = (std::min)(clip.y + clip.h, parentClip.y + parentClip.h);
            if (right <= left || bottom <= top) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
                    "[GetClipRect] Intersection is empty - returning active zero-size clip");
                return NeoWPF::Rendering::ClipRect{left, top, 0.0f, 0.0f, true};
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
                "[GetClipRect] Intersected clip: (%g, %g, %g, %g)", left, top, right - left, bottom - top);
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

    if (auto scaleTransform = std::dynamic_pointer_cast<const NeoWPF::UI::Core::ScaleTransform>(transform)) {
        const float scaleX = scaleTransform->GetScaleX();
        const float scaleY = scaleTransform->GetScaleY();

        const float pivotX = x + originX;
        const float pivotY = y + originY;

        x = pivotX + (x - pivotX) * scaleX;
        y = pivotY + (y - pivotY) * scaleY;
        width *= scaleX;
        height *= scaleY;
        return;
    }

    if (auto transformGroup = std::dynamic_pointer_cast<const NeoWPF::UI::Core::TransformGroup>(transform)) {
        for (size_t i = 0; i < transformGroup->GetChildCount(); ++i) {
            const auto& child = transformGroup->GetChild(i);
            if (!child) {
                continue;
            }

            if (auto childScale = std::dynamic_pointer_cast<const NeoWPF::UI::Core::ScaleTransform>(child)) {
                const float scaleX = childScale->GetScaleX();
                const float scaleY = childScale->GetScaleY();

                const float pivotX = x + originX;
                const float pivotY = y + originY;

                x = pivotX + (x - pivotX) * scaleX;
                y = pivotY + (y - pivotY) * scaleY;
                width *= scaleX;
                height *= scaleY;
                continue;
            }

            const NeoWPF::UI::Core::Matrix4 childMatrix = child->GetMatrix();
            const float translateX = childMatrix.m[3][0];
            const float translateY = childMatrix.m[3][1];

            const float absoluteOriginX = x + originX;
            const float absoluteOriginY = y + originY;

            const auto applyPoint = [&](float px, float py, float& outX, float& outY) {
                const float dx = px - absoluteOriginX;
                const float dy = py - absoluteOriginY;
                outX = childMatrix.m[0][0] * dx + childMatrix.m[0][1] * dy + absoluteOriginX + translateX;
                outY = childMatrix.m[1][0] * dx + childMatrix.m[1][1] * dy + absoluteOriginY + translateY;
            };

            float transformedOriginX = absoluteOriginX;
            float transformedOriginY = absoluteOriginY;
            applyPoint(absoluteOriginX, absoluteOriginY, transformedOriginX, transformedOriginY);

            x = transformedOriginX - originX;
            y = transformedOriginY - originY;
        }
        return;
    }

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

NeoWPF::UI::Core::Matrix4 FrameworkElement::GetLocalTransformMatrix() const noexcept
{
    const auto origin = GetRenderTransformOrigin();
    const float width = static_cast<float>((std::max)(GetFinalWidth(), GetDesiredWidth()));
    const float height = static_cast<float>((std::max)(GetFinalHeight(), GetDesiredHeight()));
    const float originX = width * origin.X;
    const float originY = height * origin.Y;

    NeoWPF::UI::Core::Matrix4 local = NeoWPF::UI::Core::Matrix4::Translation(
        static_cast<float>(m_offsetX + m_renderOffsetX),
        static_cast<float>(m_offsetY + m_renderOffsetY));

    if (auto transform = GetRenderTransform()) {
        if (originX != 0.0f || originY != 0.0f) {
            local = NeoWPF::UI::Core::Matrix4::Translation(-originX, -originY);
            local = local * transform->GetMatrix();
            local = local * NeoWPF::UI::Core::Matrix4::Translation(originX, originY);
            local = local * NeoWPF::UI::Core::Matrix4::Translation(
                static_cast<float>(m_offsetX + m_renderOffsetX),
                static_cast<float>(m_offsetY + m_renderOffsetY));
        } else {
            local = transform->GetMatrix() * local;
        }
    }

    return local;
}

}
