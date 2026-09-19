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
        double absX = 0.0;
        double absY = 0.0;
        GetAbsolutePosition(absX, absY);

        clip.x = absX;
        clip.y = absY;
        clip.w = m_finalWidth;
        clip.h = m_finalHeight;
        clip.active = true;

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
            "[GetClipRect] Element clip ACTIVE: (%g, %g, %g, %g)", clip.x, clip.y, clip.w, clip.h);
    }

    FrameworkElement* parent = nullptr;
    for (auto* current = GetParent(); current && !parent; current = current->GetParent()) {
        parent = dynamic_cast<FrameworkElement*>(current);
    }
    if (parent) {
        const auto parentClip = parent->GetClipRect();
        if (parentClip.active) {
            if (!clip.active) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Trace,
                    "[GetClipRect] Inheriting parent clip: (%g, %g, %g, %g)", parentClip.x, parentClip.y, parentClip.w, parentClip.h);
                return parentClip;
            }

            const double left = (std::max)(clip.x, parentClip.x);
            const double top = (std::max)(clip.y, parentClip.y);
            const double right = (std::min)(clip.x + clip.w, parentClip.x + parentClip.w);
            const double bottom = (std::min)(clip.y + clip.h, parentClip.y + parentClip.h);
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

void FrameworkElement::GetAbsolutePosition(double& outX, double& outY) const noexcept
{
    double x = m_offsetX + m_renderOffsetX;
    double y = m_offsetY + m_renderOffsetY;

    auto parent = GetParent();
    while (parent)
    {
        auto fw = dynamic_cast<FrameworkElement*>(parent);
        if (fw)
        {
            /* @note
               do not use desired size here. the render transform pivot must use the final
               arranged size, otherwise the pivot can be different from the size actually
               used when rendering the element.

               for example, if the desired width is larger than the final width, the pivot
               will be shifted and the element's transform can stop lining up with its
               rendered background or child content.
            */
            double width = fw->GetFinalWidth();
            double height = fw->GetFinalHeight();

            x += fw->GetLayoutOffsetX() + fw->GetRenderOffsetX();
            y += fw->GetLayoutOffsetY() + fw->GetRenderOffsetY();
            fw->ApplyRenderTransform(x, y, width, height);
        }
        parent = parent->GetParent();
    }

    outX = x;
    outY = y;
}

void FrameworkElement::GetAbsoluteLayoutPosition(double& outX, double& outY) const noexcept
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

void FrameworkElement::ApplyRenderTransform(double& x, double& y, double& width, double& height) const noexcept
{
    auto transform = GetRenderTransform();
    if (!transform) {
        return;
    }

    const auto origin = GetRenderTransformOrigin();
    const double originX = width * origin.X;
    const double originY = height * origin.Y;

    if (auto scaleTransform = std::dynamic_pointer_cast<const NeoWPF::UI::Core::ScaleTransform>(transform)) {
        const double scaleX = scaleTransform->GetScaleX();
        const double scaleY = scaleTransform->GetScaleY();

        const double pivotX = x + originX;
        const double pivotY = y + originY;

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
                const double scaleX = childScale->GetScaleX();
                const double scaleY = childScale->GetScaleY();

                const double pivotX = x + originX;
                const double pivotY = y + originY;

                x = pivotX + (x - pivotX) * scaleX;
                y = pivotY + (y - pivotY) * scaleY;
                width *= scaleX;
                height *= scaleY;
                continue;
            }

            const NeoWPF::UI::Core::Matrix4 childMatrix = child->GetMatrix();
            const double translateX = childMatrix.m[3][0];
            const double translateY = childMatrix.m[3][1];

            const double absoluteOriginX = x + originX;
            const double absoluteOriginY = y + originY;

            const auto applyPoint = [&](double px, double py, double& outX, double& outY) {
                const double dx = px - absoluteOriginX;
                const double dy = py - absoluteOriginY;
                outX = childMatrix.m[0][0] * dx + childMatrix.m[0][1] * dy + absoluteOriginX + translateX;
                outY = childMatrix.m[1][0] * dx + childMatrix.m[1][1] * dy + absoluteOriginY + translateY;
            };

            double transformedOriginX = absoluteOriginX;
            double transformedOriginY = absoluteOriginY;
            applyPoint(absoluteOriginX, absoluteOriginY, transformedOriginX, transformedOriginY);

            x = transformedOriginX - originX;
            y = transformedOriginY - originY;
        }
        return;
    }

    const NeoWPF::UI::Core::Matrix4 matrix = transform->GetMatrix();
    const double translateX = matrix.m[3][0];
    const double translateY = matrix.m[3][1];

    const double absoluteOriginX = x + originX;
    const double absoluteOriginY = y + originY;

    const auto applyPoint = [&](double px, double py, double& outX, double& outY) {
        const double dx = px - absoluteOriginX;
        const double dy = py - absoluteOriginY;
        outX = matrix.m[0][0] * dx + matrix.m[0][1] * dy + absoluteOriginX + translateX;
        outY = matrix.m[1][0] * dx + matrix.m[1][1] * dy + absoluteOriginY + translateY;
    };

    double transformedOriginX = absoluteOriginX;
    double transformedOriginY = absoluteOriginY;
    applyPoint(absoluteOriginX, absoluteOriginY, transformedOriginX, transformedOriginY);

    x = transformedOriginX - originX;
    y = transformedOriginY - originY;
}

NeoWPF::UI::Core::Matrix4 FrameworkElement::GetLocalTransformMatrix() const noexcept
{
    const auto origin = GetRenderTransformOrigin();
    /* @note
       do not use desired size here. the render transform pivot must be based on the
       final arranged size, otherwise the pivot can shift when the desired size is
       larger than the actual size of the element

       this can cause transformed content to no longer line up correctly with the
       element or its parent
    */
    const double width = GetFinalWidth();
    const double height = GetFinalHeight();
    const double originX = width * origin.X;
    const double originY = height * origin.Y;

    NeoWPF::UI::Core::Matrix4 local = NeoWPF::UI::Core::Matrix4::Translation(
        static_cast<float>(m_offsetX + m_renderOffsetX),
        static_cast<float>(m_offsetY + m_renderOffsetY));

    if (auto transform = GetRenderTransform()) {
        if (originX != 0.0f || originY != 0.0f) {
            local = NeoWPF::UI::Core::Matrix4::Translation(static_cast<float>(-originX), static_cast<float>(-originY));
            local = local * transform->GetMatrix();
            local = local * NeoWPF::UI::Core::Matrix4::Translation(static_cast<float>(originX), static_cast<float>(originY));
            local = local * NeoWPF::UI::Core::Matrix4::Translation(
                static_cast<float>(m_offsetX + m_renderOffsetX),
                static_cast<float>(m_offsetY + m_renderOffsetY));
        } else {
            local = transform->GetMatrix() * local;
        }
    }

    return local;
}

NeoWPF::UI::Core::Matrix4 FrameworkElement::GetLocalPositionMatrix() const noexcept
{
    const auto origin = GetRenderTransformOrigin();
    const double originX = GetFinalWidth() * origin.X;
    const double originY = GetFinalHeight() * origin.Y;
    const auto transform = GetRenderTransform();
    const auto positionTransform = transform ? transform->GetPositionMatrix()
                                              : NeoWPF::UI::Core::Matrix4::Identity();
    return NeoWPF::UI::Core::Matrix4::Translation(static_cast<float>(-originX), static_cast<float>(-originY))
        * positionTransform
        * NeoWPF::UI::Core::Matrix4::Translation(static_cast<float>(originX), static_cast<float>(originY))
        * NeoWPF::UI::Core::Matrix4::Translation(
            static_cast<float>(m_offsetX + m_renderOffsetX),
            static_cast<float>(m_offsetY + m_renderOffsetY));
}

}
