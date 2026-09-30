#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <algorithm>

namespace NeoWPF::Input
{
    namespace {
        bool IsInSubtree(const NeoWPF::UI::Visual* node, const NeoWPF::UI::Visual* subtreeRoot)
        {
            for (; node; node = node->GetParent()) {
                if (node == subtreeRoot) {
                    return true;
                }
            }
            return false;
        }
    }

    void InputManager::CollectFocusableElements(NeoWPF::UI::UIElement* root,
                                               std::vector<NeoWPF::UI::UIElement*>& elements) const
    {
        if (!root) {
            return;
        }

        if (root->CanFocus()) {
            elements.push_back(root);
        }

        for (auto* child : root->GetVisualChildren()) {
            if (auto* uiChild = dynamic_cast<NeoWPF::UI::UIElement*>(child)) {
                CollectFocusableElements(uiChild, elements);
            }
        }
    }

    bool InputManager::TryMoveFocus(bool forward)
    {
        if (!m_rootElement) {
            return false;
        }

        std::vector<NeoWPF::UI::UIElement*> focusables;
        CollectFocusableElements(m_rootElement, focusables);

        if (focusables.empty()) {
            return false;
        }

        if (!m_focusedElement) {
            SetFocusedElement(forward ? focusables.front() : focusables.back());
            return true;
        }

        const auto it = std::find(focusables.begin(), focusables.end(), m_focusedElement);
        if (it == focusables.end()) {
            SetFocusedElement(forward ? focusables.front() : focusables.back());
            return true;
        }

        const std::size_t currentIndex = static_cast<std::size_t>(std::distance(focusables.begin(), it));
        const std::size_t nextIndex = forward
            ? (currentIndex + 1) % focusables.size()
            : (currentIndex == 0 ? focusables.size() - 1 : currentIndex - 1);

        if (focusables[currentIndex] == focusables[nextIndex]) {
            return true;
        }

        SetFocusedElement(focusables[nextIndex]);
        return true;
    }

    InputManager& InputManager::GetInstance()
    {
        static InputManager instance;
        return instance;
    }

    void InputManager::SetRootElement(NeoWPF::UI::UIElement* root)
    {
        if (m_rootElement == root)
        {
            return;
        }

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
            "[Input] Root changed: old=%p new=%p",
            static_cast<void*>(m_rootElement), static_cast<void*>(root));

        if (m_focusedElement)
        {
            m_focusedElement->OnLostFocus();
            m_focusedElement = nullptr;
        }

        m_captureElement = nullptr;
        m_mouseOverElement = nullptr;
        m_lastHitTestResult = nullptr;
        m_rootElement = root;
    }

    void InputManager::NotifyVisualTreeReplaced(NeoWPF::UI::Visual* oldRoot)
    {
        if (!oldRoot) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
                "[Input] Full visual-tree reset: root=%p hover=%p focus=%p capture=%p",
                static_cast<void*>(m_rootElement), static_cast<void*>(m_mouseOverElement),
                static_cast<void*>(m_focusedElement), static_cast<void*>(m_captureElement));
            ++m_treeEpoch;
            if (m_focusedElement) {
                m_focusedElement->SetFocused(false);
                m_focusedElement->OnLostFocus();
            }
            m_focusedElement = nullptr;
            m_captureElement = nullptr;
            m_mouseOverElement = nullptr;
            m_lastHitTestResult = nullptr;
            m_rootElement = nullptr;
            return;
        }

        const auto inOld = [oldRoot](const NeoWPF::UI::UIElement* element) {
            return element && IsInSubtree(element, oldRoot);
        };
        const bool hover = inOld(m_mouseOverElement);
        const bool focus = inOld(m_focusedElement);
        const bool capture = inOld(m_captureElement);
        const bool root = inOld(m_rootElement);
        if (!hover && !focus && !capture && !root) {
            return;
        }

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
            "[Input] Visual tree replaced: oldRoot=%p hover=%d focus=%d capture=%d root=%d",
            static_cast<void*>(oldRoot), hover, focus, capture, root);

        if (hover) {
            auto* previous = m_mouseOverElement;
            m_mouseOverElement = nullptr;
            Core::MouseEventArgs args(m_mouseX, m_mouseY, m_mouseButtonState, 0);
            previous->OnMouseLeave(&args);
            RouteMouseEvent(previous, Core::MouseLeaveEvent(), m_mouseX, m_mouseY, m_mouseButtonState, 0);
        }
        ++m_treeEpoch;
        if (focus) {
            auto* focused = m_focusedElement;
            m_focusedElement = nullptr;
            focused->SetFocused(false);
            focused->OnLostFocus();
        }
        if (capture) {
            m_captureElement = nullptr;
        }
        if (root) {
            m_rootElement = nullptr;
            m_overlayElements.clear();
        }
        if (inOld(m_lastHitTestResult)) {
            m_lastHitTestResult = nullptr;
        }
    }

    void InputManager::SetOverlayElements(const std::vector<std::shared_ptr<NeoWPF::UI::Visual>>& overlays)
    {
        m_overlayElements = overlays;
    }

    void InputManager::ClickAt(NeoWPF::UI::UIElement* target, int x, int y, std::uint32_t buttonState)
    {
        if (!target) {
            return;
        }

        const auto treeEpoch = m_treeEpoch;
        const auto treeChanged = [this, treeEpoch]() { return m_treeEpoch != treeEpoch; };
        std::shared_ptr<NeoWPF::UI::Visual> keepAlive;
        try {
            keepAlive = target->shared_from_this();
        } catch (const std::bad_weak_ptr&) {
        }

        RouteMouseEvent(target, Core::PreviewMouseDownEvent(), x, y, buttonState, 0);
        if (treeChanged()) return;
        target->OnMouseDown(x, y, buttonState);
        if (treeChanged()) return;
        RouteMouseEvent(target, Core::MouseDownEvent(), x, y, buttonState, 0);
        if (treeChanged()) return;

        RouteMouseEvent(target, Core::PreviewMouseUpEvent(), x, y, buttonState, 0);
        if (treeChanged()) return;
        target->OnMouseUp(x, y, buttonState);
        if (treeChanged()) return;
        RouteMouseEvent(target, Core::MouseUpEvent(), x, y, buttonState, 0);
    }

    void InputManager::ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta)
    {
        NEO_WPF_ACTION("Input.ProcessMouseInput");

        m_mouseX = x;
        m_mouseY = y;
        const auto treeEpoch = m_treeEpoch;
        const auto treeChanged = [this, treeEpoch]() { return m_treeEpoch != treeEpoch; };

        if (m_captureElement)
        {
            auto* captured = m_captureElement;
            std::shared_ptr<NeoWPF::UI::Visual> keepAlive;
            try {
                keepAlive = captured->shared_from_this();
            } catch (const std::bad_weak_ptr&) {
            }
            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            if ((buttonState & 0x01) != (m_prevMouseButtonState & 0x01))
            {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
                    "[Input] Captured mouse button %s: target=%p position=(%d,%d) buttons=%u",
                    (buttonState & 0x01) != 0 ? "down" : "up", static_cast<void*>(captured), x, y, buttonState);
                if ((buttonState & 0x01) != 0)
                {
                    RouteMouseEvent(captured, Core::PreviewMouseDownEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    captured->OnMouseDown(x, y, buttonState);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    RouteMouseEvent(captured, Core::MouseDownEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                }
                else
                {
                    RouteMouseEvent(captured, Core::PreviewMouseUpEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    captured->OnMouseUp(x, y, buttonState);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    RouteMouseEvent(captured, Core::MouseUpEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                }
            }
            RouteMouseEvent(captured, Core::PreviewMouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (treeChanged()) {
                m_mouseButtonState = m_prevMouseButtonState = buttonState;
                return;
            }
            if (wheelDelta != 0)
            {
                RouteMouseEvent(captured, Core::PreviewMouseWheelEvent(), x, y, buttonState, wheelDelta);
                if (treeChanged()) {
                    m_mouseButtonState = m_prevMouseButtonState = buttonState;
                    return;
                }
            }
            if (captured)
            {
                captured->OnMouseInput(&args);
            }
            RouteMouseEvent(captured, Core::MouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (treeChanged()) {
                m_mouseButtonState = m_prevMouseButtonState = buttonState;
                return;
            }
            if (wheelDelta != 0)
            {
                RouteMouseEvent(captured, Core::MouseWheelEvent(), x, y, buttonState, wheelDelta);
                if (treeChanged()) {
                    m_mouseButtonState = m_prevMouseButtonState = buttonState;
                    return;
                }
            }
            m_mouseButtonState = buttonState;
            m_prevMouseButtonState = buttonState;
            return;
        }

        NeoWPF::UI::UIElement* targetElement = HitTest(x, y);

        if ((buttonState & 0x01) != (m_prevMouseButtonState & 0x01)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
                "[Input] Mouse button %s: target=%p position=(%d,%d) buttons=%u",
                (buttonState & 0x01) != 0 ? "down" : "up", static_cast<void*>(targetElement), x, y, buttonState);
        }

        if (targetElement != m_mouseOverElement)
        {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
                "[Input] Hover changed: old=%p new=%p position=(%d,%d)",
                static_cast<void*>(m_mouseOverElement), static_cast<void*>(targetElement), x, y);
            if (m_mouseOverElement)
            {
                Core::MouseEventArgs leaveArgs(x, y, buttonState, wheelDelta);
                m_mouseOverElement->OnMouseLeave(&leaveArgs);
                RouteMouseEvent(m_mouseOverElement, Core::MouseLeaveEvent(), x, y, buttonState, wheelDelta);
            }
            if (targetElement)
            {
                Core::MouseEventArgs enterArgs(x, y, buttonState, wheelDelta);
                targetElement->OnMouseEnter(&enterArgs);
                RouteMouseEvent(targetElement, Core::MouseEnterEvent(), x, y, buttonState, wheelDelta);
            }
            m_mouseOverElement = targetElement;
        }

        if (targetElement)
        {
            std::shared_ptr<NeoWPF::UI::Visual> keepAlive;
            try {
                keepAlive = targetElement->shared_from_this();
            } catch (const std::bad_weak_ptr&) {
            }
            if ((buttonState & 0x01) != (m_prevMouseButtonState & 0x01))
            {
                if ((buttonState & 0x01) != 0)
                {
                    RouteMouseEvent(targetElement, Core::PreviewMouseDownEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    targetElement->OnMouseDown(x, y, buttonState);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    RouteMouseEvent(targetElement, Core::MouseDownEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    SetFocusedElement(targetElement);
                }
                else
                {
                    RouteMouseEvent(targetElement, Core::PreviewMouseUpEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    targetElement->OnMouseUp(x, y, buttonState);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                    RouteMouseEvent(targetElement, Core::MouseUpEvent(), x, y, buttonState, wheelDelta);
                    if (treeChanged()) {
                        m_mouseButtonState = m_prevMouseButtonState = buttonState;
                        return;
                    }
                }
            }

            RouteMouseEvent(targetElement, Core::PreviewMouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (treeChanged()) {
                m_mouseButtonState = m_prevMouseButtonState = buttonState;
                return;
            }
            if (wheelDelta != 0)
            {
                RouteMouseEvent(targetElement, Core::PreviewMouseWheelEvent(), x, y, buttonState, wheelDelta);
                if (treeChanged()) {
                    m_mouseButtonState = m_prevMouseButtonState = buttonState;
                    return;
                }
            }
            Core::MouseEventArgs args(x, y, buttonState, wheelDelta);
            targetElement->OnMouseInput(&args);
            RouteMouseEvent(targetElement, Core::MouseMoveEvent(), x, y, buttonState, wheelDelta);
            if (treeChanged()) {
                m_mouseButtonState = m_prevMouseButtonState = buttonState;
                return;
            }
            if (wheelDelta != 0)
            {
                RouteMouseEvent(targetElement, Core::MouseWheelEvent(), x, y, buttonState, wheelDelta);
                if (treeChanged()) {
                    m_mouseButtonState = m_prevMouseButtonState = buttonState;
                    return;
                }
            }
        }

        m_mouseButtonState = buttonState;
        m_prevMouseButtonState = buttonState;
    }

    void InputManager::ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
    {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
            "[Input] Key %s: key=%d char=%u repeat=%d focus=%p",
            isKeyDown ? "down" : "up", keyCode, static_cast<unsigned int>(charCode), repeatCount,
            static_cast<void*>(m_focusedElement));
        if (keyCode >= 0 && keyCode < static_cast<int>(m_keyStates.size()))
        {
            m_keyStates[keyCode] = isKeyDown;
        }

        if (keyCode == 0x09 && isKeyDown)
        {
            const bool shiftPressed = IsKeyDown(0x10);
            TryMoveFocus(!shiftPressed);
            return;
        }

        if (!m_focusedElement)
        {
            return;
        }

        auto& eventDef = isKeyDown ? Core::PreviewKeyDownEvent() : Core::PreviewKeyUpEvent();
        RouteKeyboardEvent(m_focusedElement, eventDef, keyCode, isKeyDown, charCode, repeatCount);

        Core::KeyboardEventArgs args(keyCode, isKeyDown, charCode, repeatCount);
        m_focusedElement->OnKeyboardInput(&args);

        RouteKeyboardEvent(m_focusedElement, isKeyDown ? Core::KeyDownEvent() : Core::KeyUpEvent(),
                           keyCode, isKeyDown, charCode, repeatCount);
    }

    bool InputManager::IsKeyDown(int keyCode) const
    {
        if (keyCode < 0 || keyCode >= static_cast<int>(m_keyStates.size()))
        {
            return false;
        }
        return m_keyStates[keyCode];
    }

    bool InputManager::IsKeyUp(int keyCode) const
    {
        return !IsKeyDown(keyCode);
    }
}
