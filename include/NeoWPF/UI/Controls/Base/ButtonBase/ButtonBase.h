#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::Input::Core {
    class MouseEventArgs;
}

namespace NeoWPF::UI {

class ButtonBase : public NeoWPF::UI::Content::ContentControl {
public:
    ButtonBase();
    virtual ~ButtonBase() = default;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    NeoWPF::Core::Event<> Click;

protected:
    void InitializeVisualStates();
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);
    void RaiseClick();
    virtual void OnVisualStateApplied(const std::string& stateName);

protected:
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;
};

}
