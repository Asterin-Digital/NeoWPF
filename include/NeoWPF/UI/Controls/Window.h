#pragma once

#include <NeoWPF/UI/Core/FrameworkElement.h>

namespace NeoWPF::UI {

class Window : public FrameworkElement {
public:
    void SetTitle(const wchar_t* title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title; }

private:
    const wchar_t* m_title = L"NeoWPF";
};

}
