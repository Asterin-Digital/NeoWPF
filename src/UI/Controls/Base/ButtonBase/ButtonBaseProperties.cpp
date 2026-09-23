#include <NeoWPF/UI/Controls/Base/ButtonBase/ButtonBase.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeCommandParameterMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender,
                                          const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
        if (auto button = dynamic_cast<NeoWPF::UI::ButtonBase*>(sender)) {
            button->SetCommandParameter(args.NewValue);
        }
    };
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterButtonBaseProperties()
{
    RegisterProperty<UI::ButtonBase, std::shared_ptr<NeoWPF::Input::ICommand>>(
        "Command", &UI::ButtonBase::GetCommand, &UI::ButtonBase::SetCommand);
    RegisterProperty<UI::ButtonBase, std::any>(
        "CommandParameter", &UI::ButtonBase::GetCommandParameter, &UI::ButtonBase::SetCommandParameter,
        MakeCommandParameterMetadata());
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
            &NeoWPF::UI::ButtonBase::SetCommandParameter,
            MakeCommandParameterMetadata());
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