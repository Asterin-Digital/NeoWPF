#include <NeoWPF/UI/Localization/LocaleString.h>

void NeoWPF::UI::Localization::LocaleString::SetKey(const std::wstring& key)
{
    if (!IsApplyingPropertyValue()) {
        Properties::DependencyObject::SetValue(KeyProperty(), key);
        return;
    }
    m_key = key;
}

void NeoWPF::UI::Localization::LocaleString::SetValue(const std::wstring& value)
{
    if (!IsApplyingPropertyValue()) {
        Properties::DependencyObject::SetValue(ValueProperty(), value);
        return;
    }
    m_value = value;
}