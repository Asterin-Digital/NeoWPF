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

class ToggleBase : public NeoWPF::UI::Content::ContentControl {
public:
    ToggleBase();
    virtual ~ToggleBase() = default;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    bool IsChecked() const noexcept { return m_isChecked; }
    void SetChecked(bool value) noexcept;
    void Toggle() noexcept;

    void SetContentText(const std::wstring& text);
    const std::wstring& GetContentText() const noexcept { return m_contentText; }

    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    NeoWPF::Core::Event<> Checked;
    NeoWPF::Core::Event<> Unchecked;
    NeoWPF::Core::Event<> Click;

protected:
    void OnInitialize() override;
    void InitializeVisualStates();
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);
    void RaiseChecked();
    void RaiseUnchecked();
    void RaiseClick();
    virtual void OnVisualStateApplied(const std::string& stateName);
    virtual void UpdateContentTextFromState();

protected:
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;
    bool m_isChecked = false;
    std::wstring m_contentText;
};

}
