#pragma once

#include <NeoWPF/UI/Styling/State/VisualStateManager.h>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI {
class UIElement;
class Visual;
}

namespace NeoWPF::UI::Templates {

struct TemplatePart {
    std::string Name;
    bool Required = true;
};

class ControlTemplateBase {
public:
    virtual ~ControlTemplateBase() = default;
    virtual bool CanApplyTo(UIElement* element) const;
    virtual void Apply(UIElement* element) = 0;
    virtual std::shared_ptr<Visual> GetVisualTreeRoot() const noexcept { return nullptr; }
    virtual std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetVisualStateManager() const noexcept { return nullptr; }

    void AddRequiredPart(const std::string& name, bool required = true);
    bool HasRequiredParts() const noexcept;
    const std::vector<TemplatePart>& GetRequiredParts() const noexcept;

protected:
    std::vector<TemplatePart> m_requiredParts;
};

class ControlTemplate : public ControlTemplateBase {
public:
    explicit ControlTemplate();

    bool CanApplyTo(UIElement* element) const override;
    void Apply(UIElement* element) override;
    std::shared_ptr<Visual> GetVisualTreeRoot() const noexcept override;
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetVisualStateManager() const noexcept override;

    void SetVisualStateManager(const std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager>& visualStateManager);
    void AddVisualStateGroup(const std::shared_ptr<NeoWPF::UI::Styling::VisualStateGroup>& group);
    void AddVisualState(const std::shared_ptr<NeoWPF::UI::Styling::VisualState>& state, const std::string& groupName = "CommonStates");

    void SetVisualTree(std::shared_ptr<Visual> visualTree);
    std::shared_ptr<Visual> GetVisualTree() const noexcept;

    void AddVisualChild(std::shared_ptr<Visual> visualChild);
    void AddTemplateChild(const std::string& name, std::shared_ptr<Visual> visualChild);

private:
    struct TemplateChild {
        std::string Name;
        std::shared_ptr<Visual> Visual;
    };

    std::shared_ptr<Visual> m_visualTree;
    std::vector<TemplateChild> m_templateChildren;
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
};

} // namespace NeoWPF::UI::Templates
