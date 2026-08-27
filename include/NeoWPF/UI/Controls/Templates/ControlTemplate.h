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

/*
    @summary
    Describes a named template part.
*/
struct TemplatePart {
    std::string Name;
    bool Required = true;
};

class ControlTemplateBase {
public:
    virtual ~ControlTemplateBase() = default;

    /*
        @summary
        Checks whether the template can be applied to an element.

        @param element
        The element that will receive the template.

        @returns
        True when the template is compatible with the element.

        @internal
    */
    virtual bool CanApplyTo(UIElement* element) const;
    /*
        @summary
        Applies the template to an element.

        @param element
        The element that receives the template.

        @internal
    */
    virtual void Apply(UIElement* element) = 0;

    /*
        @summary
        Returns the template root visual.

        @returns
        The root visual, if present.

        @internal
    */
    virtual std::shared_ptr<Visual> GetVisualTreeRoot() const noexcept { return nullptr; }
    /*
        @summary
        Returns the visual state manager for the template.

        @returns
        The state manager, if present.

        @internal
    */
    virtual std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetVisualStateManager() const noexcept { return nullptr; }

    /*
        @summary
        Adds a required template part.

        @param name
        The part name.

        @param required
        Whether the part must be present.

        @internal
    */
    void AddRequiredPart(const std::string& name, bool required = true);
    /*
        @summary
        Checks whether the template has required parts.

        @returns
        True when at least one required part is registered.

        @internal
    */
    bool HasRequiredParts() const noexcept;

    /*
        @summary
        Returns the required parts for the template.

        @returns
        The registered template parts.

        @internal
    */
    const std::vector<TemplatePart>& GetRequiredParts() const noexcept;

protected:
    std::vector<TemplatePart> m_requiredParts;
};

class ControlTemplate : public ControlTemplateBase {
public:
    /*
        @summary
        Creates a new control template.
    */
    explicit ControlTemplate();

    /*
        @summary
        Checks whether the template can be applied to an element.

        @param element
        The element that will receive the template.

        @returns
        True when the template is compatible with the element.
    */
    bool CanApplyTo(UIElement* element) const override;
    /*
        @summary
        Applies the template to an element.

        @param element
        The element that receives the template.
    */
    void Apply(UIElement* element) override;

    /*
        @summary
        Returns the template root visual.

        @returns
        The root visual, if present.

        @internal
    */
    std::shared_ptr<Visual> GetVisualTreeRoot() const noexcept override;
    /*
        @summary
        Returns the visual state manager for the template.

        @returns
        The state manager, if present.

        @internal
    */
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> GetVisualStateManager() const noexcept override;

    /*
        @summary
        Sets the visual state manager for the template.

        @param visualStateManager
        The state manager to use.
    */
    void SetVisualStateManager(const std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager>& visualStateManager);
    /*
        @summary
        Adds a visual state group to the template.

        @param group
        The group to register.
    */
    void AddVisualStateGroup(const std::shared_ptr<NeoWPF::UI::Styling::VisualStateGroup>& group);

    /*
        @summary
        Adds a visual state to a group.

        @param state
        The state to register.

        @param groupName
        The target group name.
    */
    void AddVisualState(const std::shared_ptr<NeoWPF::UI::Styling::VisualState>& state, const std::string& groupName = "CommonStates");

    /*
        @summary
        Sets the root visual for the template.

        @param visualTree
        The visual to use as the root.
    */
    void SetVisualTree(std::shared_ptr<Visual> visualTree);
    /*
        @summary
        Returns the template root visual.

        @returns
        The root visual.
    */
    std::shared_ptr<Visual> GetVisualTree() const noexcept;

    /*
        @summary
        Adds a visual child to the template.

        @param visualChild
        The child visual to add.
    */
    void AddVisualChild(std::shared_ptr<Visual> visualChild);
    /*
        @summary
        Adds a named template child.

        @param name
        The template part name.

        @param visualChild
        The child visual to register.
    */
    void AddTemplateChild(const std::string& name, std::shared_ptr<Visual> visualChild);

private:
    struct TemplateChild {
        std::string Name;
        std::shared_ptr<NeoWPF::UI::Visual> Visual;
    };

    std::shared_ptr<Visual> m_visualTree;
    std::vector<TemplateChild> m_templateChildren;
    std::shared_ptr<NeoWPF::UI::Styling::VisualStateManager> m_visualStateManager;
};

} 
