#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterButtonBaseProperties()
{
    RegisterProperty<UI::ButtonBase, std::shared_ptr<NeoWPF::Input::ICommand>>(
        "Command", &UI::ButtonBase::GetCommand, &UI::ButtonBase::SetCommand);
    RegisterProperty<UI::ButtonBase, std::any>(
        "CommandParameter", &UI::ButtonBase::GetCommandParameter, &UI::ButtonBase::SetCommandParameter);
    RegisterProperty<UI::ButtonBase, NeoWPF::UI::UIElement*>(
        "CommandTarget", &UI::ButtonBase::GetCommandTarget, &UI::ButtonBase::SetCommandTarget);
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ButtonBase::CommandProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ButtonBase, std::shared_ptr<NeoWPF::Input::ICommand>>(
            "Command", &NeoWPF::UI::ButtonBase::GetCommand, &NeoWPF::UI::ButtonBase::SetCommand);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ButtonBase::CommandParameterProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ButtonBase, std::any>(
            "CommandParameter", &NeoWPF::UI::ButtonBase::GetCommandParameter,
            &NeoWPF::UI::ButtonBase::SetCommandParameter);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ButtonBase::CommandTargetProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ButtonBase, NeoWPF::UI::UIElement*>(
            "CommandTarget", &NeoWPF::UI::ButtonBase::GetCommandTarget,
            &NeoWPF::UI::ButtonBase::SetCommandTarget);
    return *property;
}