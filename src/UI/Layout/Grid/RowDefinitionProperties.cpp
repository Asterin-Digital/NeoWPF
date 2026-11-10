#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/GridDefinitionConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <algorithm>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeRowDefinitionHeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Grid::GridLength::Star());
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        metadata.Converter = NeoWPF::UI::Properties::GetGridLengthConverter();
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterRowDefinitionProperties() {
        RegisterProperty<NeoWPF::UI::RowDefinition, NeoWPF::UI::Grid::GridLength>("Height", MakeRowDefinitionHeightMetadata());
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::HeightProperty() {
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<NeoWPF::UI::RowDefinition, NeoWPF::UI::Grid::GridLength>("Height", MakeRowDefinitionHeightMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RowDefinition::ModeProperty() { return HeightProperty(); }

void NeoWPF::UI::RowDefinition::SetHeight(NeoWPF::UI::Grid::GridLength height) {
    SetValue(HeightProperty(), height);
}

NeoWPF::UI::Grid::GridLength NeoWPF::UI::RowDefinition::GetHeight() const {
    return GetValue<NeoWPF::UI::Grid::GridLength>(HeightProperty());
}

void NeoWPF::UI::RowDefinition::SetMode(int mode) {
    SetHeight({static_cast<NeoWPF::UI::SizeMode>(mode), 0});
}

int NeoWPF::UI::RowDefinition::GetMode() const {
    return static_cast<int>(GetHeight().Mode);
}

void NeoWPF::UI::RowDefinition::OnDependencyPropertyChanged(
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
            owner->owner->OnRowDefinitionChanged(this);
        }
    }
}

void NeoWPF::UI::RowDefinition::AddOwner(
    const std::shared_ptr<NeoWPF::UI::GridDefinitionOwner>& owner)
{
    if (owner && std::find(m_owners.begin(), m_owners.end(), owner) == m_owners.end()) {
        m_owners.reserve(m_owners.size() + 1);
        m_owners.push_back(owner);
    }
}

void NeoWPF::UI::RowDefinition::RemoveOwner(
    const std::shared_ptr<NeoWPF::UI::GridDefinitionOwner>& owner) noexcept
{
    m_owners.erase(std::remove(m_owners.begin(), m_owners.end(), owner), m_owners.end());
}
