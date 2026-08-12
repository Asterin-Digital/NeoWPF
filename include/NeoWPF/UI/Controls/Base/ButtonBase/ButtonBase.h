#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <any>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <NeoWPF/Input/Command.h>

namespace NeoWPF::Input::Core {
    class MouseEventArgs;
}

namespace NeoWPF::UI {

class ButtonBase : public NeoWPF::UI::Content::ContentControl, public NeoWPF::Input::ICommandSource {
public:
    ButtonBase();
    virtual ~ButtonBase() = default;

    bool IsHitTestVisible() const noexcept override { return true; }
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    virtual void OnMouseEnter(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseLeave(NeoWPF::Input::Core::MouseEventArgs* args) override;
    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) override;
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    std::shared_ptr<NeoWPF::Input::ICommand> GetCommand() const noexcept override { return m_command; }
    void SetCommand(std::shared_ptr<NeoWPF::Input::ICommand> command) override;

    std::any GetCommandParameter() const noexcept override { return m_commandParameter; }
    void SetCommandParameter(const std::any& parameter) override { m_commandParameter = parameter; }

    NeoWPF::UI::UIElement* GetCommandTarget() const noexcept override { return m_commandTarget; }
    void SetCommandTarget(NeoWPF::UI::UIElement* target) noexcept override { m_commandTarget = target; }

    NeoWPF::Core::Event<> Click;

protected:
    void OnInitialize() override;
    void InitializeVisualStates();
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);
    void RaiseClick();
    virtual void OnVisualStateApplied(const std::string& stateName);

protected:
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;
    std::shared_ptr<NeoWPF::Input::ICommand> m_command;
    std::any m_commandParameter;
    NeoWPF::UI::UIElement* m_commandTarget = nullptr;
};

}
