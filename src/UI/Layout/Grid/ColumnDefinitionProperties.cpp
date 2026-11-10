#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <algorithm>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeColumnDefinitionWidthMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Grid::GridLength::Star());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridLengthConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterColumnDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::ColumnDefinition, NeoWPF::UI::Grid::GridLength>("Width", MakeColumnDefinitionWidthMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::WidthProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::ColumnDefinition, NeoWPF::UI::Grid::GridLength>("Width", MakeColumnDefinitionWidthMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ColumnDefinition::ModeProperty() { return WidthProperty(); }

void NeoWPF::UI::ColumnDefinition::SetWidth(NeoWPF::UI::Grid::GridLength width) {
    SetValue(WidthProperty(), width);
}

NeoWPF::UI::Grid::GridLength NeoWPF::UI::ColumnDefinition::GetWidth() const {
    return GetValue<NeoWPF::UI::Grid::GridLength>(WidthProperty());
}

void NeoWPF::UI::ColumnDefinition::SetMode(int mode) {
    SetWidth({static_cast<NeoWPF::UI::SizeMode>(mode), 0});
}

int NeoWPF::UI::ColumnDefinition::GetMode() const {
    return static_cast<int>(GetWidth().Mode);
}

void NeoWPF::UI::ColumnDefinition::OnDependencyPropertyChanged(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    const NeoWPF::UI::Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    NeoWPF::UI::Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
    const auto owners = m_owners;
    for (const auto& owner : owners) {
        if (owner && owner->owner) {
            owner->owner->OnColumnDefinitionChanged(this);
        }
    }
}

void NeoWPF::UI::ColumnDefinition::AddOwner(
    const std::shared_ptr<NeoWPF::UI::GridDefinitionOwner>& owner)
{
    if (owner && std::find(m_owners.begin(), m_owners.end(), owner) == m_owners.end()) {
        m_owners.reserve(m_owners.size() + 1);
        m_owners.push_back(owner);
    }
}

void NeoWPF::UI::ColumnDefinition::RemoveOwner(
    const std::shared_ptr<NeoWPF::UI::GridDefinitionOwner>& owner) noexcept
{
    m_owners.erase(std::remove(m_owners.begin(), m_owners.end(), owner), m_owners.end());
}
