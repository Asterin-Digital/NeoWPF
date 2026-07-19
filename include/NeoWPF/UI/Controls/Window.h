#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <memory>

namespace NeoWPF::UI {
class Visual;

class Window : public FrameworkElement {
public:
    void SetTitle(const wchar_t* title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title; }

    void SetContent(std::shared_ptr<Visual> content) noexcept { m_content = std::move(content); }
    std::shared_ptr<Visual> GetContent() const noexcept { return m_content; }

private:
    const wchar_t* m_title = L"NeoWPF";
    std::shared_ptr<Visual> m_content;
};

}
