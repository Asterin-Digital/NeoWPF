#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>

namespace NeoWPF::UI {

class Window : public FrameworkElement {
public:
    void SetTitle(const wchar_t* title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title; }

    void SetContent(FrameworkElement* content) {
        if (m_content == content) {
            return;
        }
        if (m_content) {
            RemoveVisualChild(m_content);
        }
        m_content = content;
        if (m_content) {
            AddChild(m_content);
        }
    }

    FrameworkElement* GetContent() const noexcept { return m_content; }

private:
    const wchar_t* m_title = L"NeoWPF";
    FrameworkElement* m_content = nullptr;
};

}
