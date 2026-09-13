/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>

namespace NeoWPF::UI::Layout::Scrolling {

class IScrollInfo {
public:
    virtual ~IScrollInfo() = default;

    virtual int GetExtentWidth() const noexcept = 0;
    virtual int GetExtentHeight() const noexcept = 0;

    virtual int GetViewportWidth() const noexcept = 0;
    virtual int GetViewportHeight() const noexcept = 0;

    virtual int GetHorizontalOffset() const noexcept = 0;
    virtual int GetVerticalOffset() const noexcept = 0;

    virtual void SetHorizontalOffset(int offset) noexcept = 0;
    virtual void SetVerticalOffset(int offset) noexcept = 0;

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
