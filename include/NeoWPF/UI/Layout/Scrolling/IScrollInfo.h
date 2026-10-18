/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>

namespace NeoWPF::UI::Layout::Scrolling {

class IScrollInfo {
public:
    virtual ~IScrollInfo() = default;

    virtual double GetExtentWidth() const noexcept = 0;
    virtual double GetExtentHeight() const noexcept = 0;

    virtual double GetViewportWidth() const noexcept = 0;
    virtual double GetViewportHeight() const noexcept = 0;

    virtual double GetHorizontalOffset() const noexcept = 0;
    virtual double GetVerticalOffset() const noexcept = 0;

    virtual void SetHorizontalOffset(double offset) noexcept = 0;
    virtual void SetVerticalOffset(double offset) noexcept = 0;

    virtual void LineLeft() noexcept = 0;
    virtual void LineRight() noexcept = 0;
    virtual void LineUp() noexcept = 0;
    virtual void LineDown() noexcept = 0;

    virtual void PageLeft() noexcept = 0;
    virtual void PageRight() noexcept = 0;
    virtual void PageUp() noexcept = 0;
    virtual void PageDown() noexcept = 0;

    virtual void MouseWheel(int delta) noexcept = 0;
};

}
