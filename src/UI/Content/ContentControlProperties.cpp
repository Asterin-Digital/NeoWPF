#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

namespace NeoWPF::UI::Properties {

const DependencyProperty& NeoWPF::UI::Content::ContentControl::ContentProperty()
{
    static const auto& property = DependencyProperty::Register<
        NeoWPF::UI::Content::ContentControl, std::any>("Content", [] {
            PropertyMetadata metadata;
            metadata.DefaultValue = std::any{};
            metadata.Converter = GetContentConverter();
            metadata.Flags = PropertyFlags::AffectsMeasure |
                PropertyFlags::AffectsArrange |
                PropertyFlags::AffectsRender;
            metadata.PropertyChangedCallback = [](DependencyObject* sender,
                                                  const DependencyPropertyChangedEventArgs& args) {
                if (auto owner = dynamic_cast<NeoWPF::UI::Content::ContentControl*>(sender)) {
                    std::any convertedValue;
                    if (TryConvertValue(
                            NeoWPF::UI::Content::ContentControl::ContentProperty(),
                            args.NewValue, convertedValue)) {
                        owner->SetContentData(convertedValue);
                    } else {
                        owner->SetContentData(args.NewValue);
                    }
                }
            };
            return metadata;
        }());
    static const bool bindingRegistered = (NeoWPF::UI::Markup::Xaml::RegisterXamlElementPropertyCallback<
        NeoWPF::UI::Content::ContentControl, NeoWPF::UI::UIElement>(property,
            [](NeoWPF::UI::Content::ContentControl& owner,
               const std::shared_ptr<NeoWPF::UI::UIElement>& content) {
                owner.SetContent(content);
            }), true);
    (void)bindingRegistered;
    return property;
}

void RegisterContentControlProperties()
{
    (void)NeoWPF::UI::Content::ContentControl::ContentProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<NeoWPF::UI::Content::ContentControl>(
        "ContentControl",
        "Content",
        "NeoWPF::UI::Content::ContentControl",
        "NeoWPF/UI/Content/ContentControl.h");
}

}