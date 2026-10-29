/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Contracts/IPointerInput.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::Input
{
    void FocusElement(NeoWPF::UI::UIElement* element);

    /*
        @summary
        Processes keyboard, focus, and hit-test input.
    */
    class InputManager : public NeoWPF::Contracts::IPointerInput
    {
    public:
        /*
            @summary
            Returns the process-wide input manager.
        */
        static InputManager& GetInstance();

        /*
            @summary
            Processes a keyboard input packet.

            @param keyCode
            Virtual key code.

            @param isKeyDown
            True when the key is pressed.

            @param charCode
            Character code associated with the packet.

            @param repeatCount
            Number of repeated key events.
        */
        void ProcessKeyboardInput(int keyCode, bool isKeyDown, wchar_t charCode = 0, int repeatCount = 1);

        /*
            @summary
            Routes a pointer packet to its captured or hit-tested target.

            @param eventArgs
            The pointer packet to route.

            @internal
        */
        void SubmitPointerEvent(const Core::PointerEventArgs& eventArgs) override;

        /*
            @summary
            Captures a pointer for the current framework root.

            @internal
        */
        bool SetPointerCapture(std::uint32_t pointerId, NeoWPF::UI::UIElement* element);

        /*
            @summary
            Returns the element that captured a pointer for the current root.

            @internal
        */
        NeoWPF::UI::UIElement* GetPointerCapture(std::uint32_t pointerId) const;

        /*
            @summary
            Releases pointer capture for the current framework root.

            @internal
        */
        void ReleasePointerCapture(std::uint32_t pointerId) noexcept;

        /*
            @summary
            Returns the topmost element at the specified coordinates.

            @param x
            Horizontal position.

            @param y
            Vertical position.
        */
        NeoWPF::UI::UIElement* HitTest(int x, int y);

        /*
            @summary
            Assigns the element that receives keyboard focus.

            @param element
            Element to focus.
        */
        void SetFocusedElement(NeoWPF::UI::UIElement* element);
        /*
            @summary
            Returns the element that currently has keyboard focus.
        */
        NeoWPF::UI::UIElement* GetFocusedElement() const { return m_focusedElement; }

        /*
            @summary
            Reports whether the specified key is pressed.

            @param keyCode
            Virtual key code to check.
        */
        bool IsKeyDown(int keyCode) const;
        /*
            @summary
            Reports whether the specified key is released.

            @param keyCode
            Virtual key code to check.
        */
        bool IsKeyUp(int keyCode) const;

        /*
            @summary
            Sets the root element used for input routing.

            @param root
            Root of the visual tree.
        */
        void SetRootElement(NeoWPF::UI::UIElement* root);
        /*
            @summary
            Updates input state after the visual tree root is replaced.

            @param oldRoot
            Previous root element, if available.
        */
        void NotifyVisualTreeReplaced(NeoWPF::UI::Visual* oldRoot = nullptr);
        /*
            @summary
            Sets the overlay visuals included in input hit testing.

            @param overlays
            Overlay visuals to use.
        */
        void SetOverlayElements(const std::vector<std::shared_ptr<NeoWPF::UI::Visual>>& overlays);
        /*
            @summary
            Moves keyboard focus to the next or previous eligible element.

            @param forward
            True to move forward through the focus order.

            @returns
            True when focus moved to another element.
        */
        bool TryMoveFocus(bool forward = true);

    private:
        InputManager() = default;
        ~InputManager() = default;

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        NeoWPF::UI::UIElement* HitTestVisual(NeoWPF::UI::Visual* visual, int x, int y, int depth = 0);
        NeoWPF::UI::UIElement* HitTestElement(NeoWPF::UI::UIElement* element, int x, int y, int depth = 0);

        void RoutePointerEvent(NeoWPF::UI::UIElement* targetElement,
                      const Core::PointerEventArgs& eventArgs);
        void RouteKeyboardEvent(NeoWPF::UI::UIElement* targetElement,
                    Core::RoutedKeyboardEventArgs& args,
                    const Core::RoutedEvent& eventDef);
        void CollectFocusableElements(NeoWPF::UI::UIElement* root,
                                     std::vector<NeoWPF::UI::UIElement*>& elements) const;

        NeoWPF::UI::UIElement* m_rootElement = nullptr;
        std::vector<std::shared_ptr<NeoWPF::UI::Visual>> m_overlayElements;
        NeoWPF::UI::UIElement* m_focusedElement = nullptr;
        std::array<bool, 256> m_keyStates{};
        using PointerKey = std::pair<NeoWPF::UI::UIElement*, std::uint32_t>;
        std::map<PointerKey, NeoWPF::UI::UIElement*> m_pointerCaptures;
        std::map<PointerKey, NeoWPF::UI::UIElement*> m_pointerOverElements;

        NeoWPF::UI::UIElement* m_lastHitTestResult = nullptr;
        std::uint64_t m_treeEpoch = 0;
    };
}
