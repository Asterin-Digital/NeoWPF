/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::Input
{
    void FocusElement(NeoWPF::UI::UIElement* element);

    /*
        @summary
        Processes mouse, keyboard, focus, and hit-test input.
    */
    class InputManager
    {
    public:
        static InputManager& GetInstance();

        void ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta = 0);
        void ClickAt(NeoWPF::UI::UIElement* target, int x, int y, std::uint32_t buttonState = 0x01);
        void ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode = 0, int repeatCount = 1);

        NeoWPF::UI::UIElement* HitTest(int x, int y);

        void SetFocusedElement(NeoWPF::UI::UIElement* element);
        NeoWPF::UI::UIElement* GetFocusedElement() const { return m_focusedElement; }

        void SetMouseCapture(NeoWPF::UI::UIElement* element);
        NeoWPF::UI::UIElement* GetMouseCapture() const { return m_captureElement; }
        NeoWPF::UI::UIElement* GetMouseOverElement() const { return m_mouseOverElement; }

        int GetMouseX() const { return m_mouseX; }
        int GetMouseY() const { return m_mouseY; }
        std::uint32_t GetMouseButtonState() const { return m_mouseButtonState; }
        bool IsKeyDown(int keyCode) const;
        bool IsKeyUp(int keyCode) const;

        void SetRootElement(NeoWPF::UI::UIElement* root);
        void NotifyVisualTreeReplaced();
        void SetOverlayElements(const std::vector<std::shared_ptr<NeoWPF::UI::Visual>>& overlays);
        bool TryMoveFocus(bool forward = true);

    private:
        InputManager() = default;
        ~InputManager() = default;

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        NeoWPF::UI::UIElement* HitTestVisual(NeoWPF::UI::Visual* visual, int x, int y, int depth = 0);
        NeoWPF::UI::UIElement* HitTestElement(NeoWPF::UI::UIElement* element, int x, int y, int depth = 0);
        void RouteMouseEvent(NeoWPF::UI::UIElement* targetElement, const Core::RoutedEvent& eventDef, 
                            int x, int y, std::uint32_t buttonState, int wheelDelta);
        void RouteKeyboardEvent(NeoWPF::UI::UIElement* targetElement, const Core::RoutedEvent& eventDef,
                               int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount);
        void CollectFocusableElements(NeoWPF::UI::UIElement* root,
                                     std::vector<NeoWPF::UI::UIElement*>& elements) const;

        NeoWPF::UI::UIElement* m_rootElement = nullptr;
        std::vector<std::shared_ptr<NeoWPF::UI::Visual>> m_overlayElements;
        NeoWPF::UI::UIElement* m_focusedElement = nullptr;
        NeoWPF::UI::UIElement* m_captureElement = nullptr;
        NeoWPF::UI::UIElement* m_mouseOverElement = nullptr;

        int m_mouseX = 0;
        int m_mouseY = 0;
        std::uint32_t m_mouseButtonState = 0;
        std::array<bool, 256> m_keyStates{};

        std::uint32_t m_prevMouseButtonState = 0;
        NeoWPF::UI::UIElement* m_lastHitTestResult = nullptr;
        std::uint64_t m_treeEpoch = 0;
    };
}
