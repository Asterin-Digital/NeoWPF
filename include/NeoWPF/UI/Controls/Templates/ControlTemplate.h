#pragma once

#include <functional>
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

    void AddRequiredPart(const std::string& name, bool required = true);
    bool HasRequiredParts() const noexcept;
    const std::vector<TemplatePart>& GetRequiredParts() const noexcept;

protected:
    std::vector<TemplatePart> m_requiredParts;
};

class ControlTemplateBuilder {
public:
    explicit ControlTemplateBuilder(UIElement* control);

    UIElement* GetControl() const noexcept;

    void RegisterPart(const std::string& name, Visual* child);
    void AttachChild(Visual* child, const std::string& partName = {});

private:
    UIElement* m_control;
};

class ControlTemplate : public ControlTemplateBase {
public:
    using Builder = std::function<void(UIElement*, ControlTemplateBuilder&)>;

    explicit ControlTemplate(Builder builder);

    bool CanApplyTo(UIElement* element) const override;
    void Apply(UIElement* element) override;

private:
    Builder m_builder;
};

} // namespace NeoWPF::UI::Templates
