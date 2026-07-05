#pragma once

// Note: do NOT set the content of the window to anything, let NeoWPF handle it
// Setting the root content to something will lead to unexpected issues with the layout

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI {

class Window : public FrameworkElement {
public:
    void SetTitle(const wchar_t* title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title; }

private:
    const wchar_t* m_title = L"NeoWPF";
};

}
