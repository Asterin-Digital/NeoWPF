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

        m_lastHitTestResult = nullptr;
        m_rootElement = root;
    }

    void InputManager::NotifyVisualTreeReplaced(NeoWPF::UI::Visual* oldRoot)
    {
        if (!oldRoot) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
                "[Input] Full visual-tree reset: root=%p focus=%p",
                static_cast<void*>(m_rootElement), static_cast<void*>(m_focusedElement));
            ++m_treeEpoch;
            if (m_focusedElement) {
                m_focusedElement->SetFocused(false);
                m_focusedElement->OnLostFocus();
            }
            m_focusedElement = nullptr;
            m_pointerCaptures.clear();
            m_pointerOverElements.clear();
            m_lastHitTestResult = nullptr;
            m_rootElement = nullptr;
            return;
        }

        const auto inOld = [oldRoot](const NeoWPF::UI::UIElement* element) {
            return element && IsInSubtree(element, oldRoot);
        };
        const bool focus = inOld(m_focusedElement);
        const bool root = inOld(m_rootElement);
        bool capture = false;
        for (auto it = m_pointerCaptures.begin(); it != m_pointerCaptures.end();) {
            if (inOld(it->first.first) || inOld(it->second)) {
                capture = true;
                it = m_pointerCaptures.erase(it);
            } else {
                ++it;
            }
        }
        bool pointerOver = false;
        for (auto it = m_pointerOverElements.begin(); it != m_pointerOverElements.end();) {
            if (inOld(it->first.first) || inOld(it->second)) {
                pointerOver = true;
                it = m_pointerOverElements.erase(it);
            } else {
                ++it;
            }
        }
        if (!focus && !root && !capture && !pointerOver) {
            return;
        }

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Input, NeoWPF::Diagnostics::Level::Debug,
            "[Input] Visual tree replaced: oldRoot=%p focus=%d capture=%d pointerOver=%d root=%d",
            static_cast<void*>(oldRoot), focus, capture, pointerOver, root);
        ++m_treeEpoch;
        if (focus) {
            auto* focused = m_focusedElement;
            m_focusedElement = nullptr;
            focused->SetFocused(false);
            focused->OnLostFocus();
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

    void InputManager::SubmitPointerEvent(const Core::PointerEventArgs& eventArgs)
    {
        const PointerKey pointerKey{m_rootElement, eventArgs.GetPointerId()};
        auto* target = static_cast<NeoWPF::UI::UIElement*>(nullptr);
        if (eventArgs.GetPhase() == Core::PointerPhase::Leave) {
            const auto pointerOver = m_pointerOverElements.find(pointerKey);
            if (pointerOver != m_pointerOverElements.end()) {
                target = pointerOver->second;
                m_pointerOverElements.erase(pointerOver);
            }
        } else {
            const auto capture = m_pointerCaptures.find(pointerKey);
            target = capture != m_pointerCaptures.end()
                ? capture->second
                : HitTest(static_cast<int>(eventArgs.GetX()), static_cast<int>(eventArgs.GetY()));
        }

        const auto send = [this, &eventArgs](NeoWPF::UI::UIElement* targetElement,
                                             Core::PointerPhase phase) {
            if (!targetElement) {
                return;
            }
            Core::PointerEventArgs routedEvent(
                eventArgs.GetPointerId(), eventArgs.GetDeviceType(), phase,
                eventArgs.GetX(), eventArgs.GetY(), eventArgs.GetButtons(),
                eventArgs.GetChangedButton(), eventArgs.GetPressure(), eventArgs.GetWheelDelta());
            RoutePointerEvent(targetElement, routedEvent);
        };

        if (eventArgs.GetPhase() == Core::PointerPhase::Move ||
            eventArgs.GetPhase() == Core::PointerPhase::Enter) {
            const auto previous = m_pointerOverElements.find(pointerKey);
            if (previous != m_pointerOverElements.end() && previous->second != target) {
                send(previous->second, Core::PointerPhase::Leave);
                m_pointerOverElements.erase(previous);
            }
            if (target) {
                const auto current = m_pointerOverElements.find(pointerKey);
                if (current == m_pointerOverElements.end()) {
                    m_pointerOverElements.emplace(pointerKey, target);
                    if (eventArgs.GetPhase() == Core::PointerPhase::Move) {
                        send(target, Core::PointerPhase::Enter);
                    }
                }
            }
        }

        send(target, eventArgs.GetPhase());
        if (eventArgs.GetPhase() == Core::PointerPhase::Down && target) {
            SetFocusedElement(target);
        }
        if (eventArgs.GetPhase() == Core::PointerPhase::Up ||
            eventArgs.GetPhase() == Core::PointerPhase::Cancel) {
            ReleasePointerCapture(eventArgs.GetPointerId());
        }
    }

    bool InputManager::SetPointerCapture(std::uint32_t pointerId,
                                         NeoWPF::UI::UIElement* element)
    {
        if (!element || !m_rootElement) {
            return false;
        }
        m_pointerCaptures[PointerKey{m_rootElement, pointerId}] = element;
        return true;
    }

    NeoWPF::UI::UIElement* InputManager::GetPointerCapture(
        std::uint32_t pointerId) const
    {
        const auto capture = m_pointerCaptures.find(PointerKey{m_rootElement, pointerId});
        return capture == m_pointerCaptures.end() ? nullptr : capture->second;
    }

    void InputManager::ReleasePointerCapture(std::uint32_t pointerId) noexcept
    {
        m_pointerCaptures.erase(PointerKey{m_rootElement, pointerId});
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
