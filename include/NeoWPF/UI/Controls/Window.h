#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <memory>

namespace NeoWPF::UI {
class Visual;

class Window : public FrameworkElement {
public:
    void SetTitle(const wchar_t* title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title; }

    void SetContent(std::shared_ptr<Visual> content) noexcept {
        m_content = std::move(content);
        RebuildVisualTree();
    }
    std::shared_ptr<Visual> GetContent() const noexcept {
        return m_visualRoot ? m_visualRoot : m_content;
    }

    void SetTitleBar(std::shared_ptr<Visual> titleBar) noexcept {
        m_titleBar = std::move(titleBar);
        RebuildVisualTree();
    }
    std::shared_ptr<Visual> GetTitleBar() const noexcept { return m_titleBar; }

private:
    void RebuildVisualTree() noexcept;

private:
    const wchar_t* m_title = L"NeoWPF";
    std::shared_ptr<Visual> m_content;
    std::shared_ptr<Visual> m_titleBar;
    std::shared_ptr<Visual> m_visualRoot;
};

}
