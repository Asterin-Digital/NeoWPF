#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::Input
{
    class InputManager
    {
    public:
        static InputManager& GetInstance();

        void ProcessMouseInput(int x, int y, std::uint32_t buttonState, int wheelDelta = 0);
        void ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode = 0, int repeatCount = 1);

        NeoWPF::UI::UIElement* HitTest(int x, int y);

        void SetFocusedElement(NeoWPF::UI::UIElement* element);
        NeoWPF::UI::UIElement* GetFocusedElement() const { return m_focusedElement; }

        void SetMouseCapture(NeoWPF::UI::UIElement* element);
        NeoWPF::UI::UIElement* GetMouseCapture() const { return m_captureElement; }

        int GetMouseX() const { return m_mouseX; }
        int GetMouseY() const { return m_mouseY; }
        std::uint32_t GetMouseButtonState() const { return m_mouseButtonState; }

        void SetRootElement(NeoWPF::UI::UIElement* root);

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

        

        NeoWPF::UI::UIElement* m_rootElement = nullptr;
        NeoWPF::UI::UIElement* m_focusedElement = nullptr;
        NeoWPF::UI::UIElement* m_captureElement = nullptr;
        NeoWPF::UI::UIElement* m_mouseOverElement = nullptr;

        int m_mouseX = 0;
        int m_mouseY = 0;
        std::uint32_t m_mouseButtonState = 0;

        std::uint32_t m_prevMouseButtonState = 0;
        NeoWPF::UI::UIElement* m_lastHitTestResult = nullptr;
    };
}
