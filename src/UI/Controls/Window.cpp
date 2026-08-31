#include <NeoWPF/UI/Controls/Window.h>

namespace NeoWPF::UI {

void Window::RebuildVisualTree() noexcept
{
    if (!m_content && !m_titleBar) {
        m_visualRoot = nullptr;
        return;
    }

    auto root = std::make_shared<Grid>();
    root->AddRow(SizeMode::Auto, 0);
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);

    if (m_titleBar) {
        root->AddChild(m_titleBar, 0, 0);
    }
    if (m_content) {
        root->AddChild(m_content, 1, 0);
    }

    m_visualRoot = root;
}

}
