#include <NeoWPF/UI/Layout/Panel/ResponsiveRule.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>

#include <cmath>
#include <utility>

namespace NeoWPF::UI {

ResponsiveRule::~ResponsiveRule()
{
    m_lifetimeGuard->store(false, std::memory_order_release);
    for (const auto& setter : GetSetters()) {
        if (!setter) {
            continue;
        }
        const auto subscription = m_setterSubscriptions.find(setter.get());
        if (subscription != m_setterSubscriptions.end()) {
            setter->RemoveChangedHandler(subscription->second);
        }
    }
}

double ResponsiveRule::GetMinWidth() const
{
    return GetValue<double>(MinWidthProperty());
}

void ResponsiveRule::SetMinWidth(double width)
{
    SetValue(MinWidthProperty(), width);
}

double ResponsiveRule::GetMaxWidth() const
{
    return GetValue<double>(MaxWidthProperty());
}

void ResponsiveRule::SetMaxWidth(double width)
{
    SetValue(MaxWidthProperty(), width);
}

double ResponsiveRule::GetMinHeight() const
{
    return GetValue<double>(MinHeightProperty());
}

void ResponsiveRule::SetMinHeight(double height)
{
    SetValue(MinHeightProperty(), height);
}

double ResponsiveRule::GetMaxHeight() const
{
    return GetValue<double>(MaxHeightProperty());
}

void ResponsiveRule::SetMaxHeight(double height)
{
    SetValue(MaxHeightProperty(), height);
}

std::vector<std::shared_ptr<Styling::Triggers::Setter>>
ResponsiveRule::GetSetters() const
{
    return GetValue<std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(
        SettersProperty());
}

void ResponsiveRule::AddSetter(
    std::shared_ptr<Styling::Triggers::Setter> setter)
{
    if (setter) {
        auto setters = GetSetters();
        if (std::find(setters.begin(), setters.end(), setter) == setters.end()) {
            auto lifetimeGuard = m_lifetimeGuard;
            const auto subscription = setter->AddChangedHandler([this, lifetimeGuard] {
                if (lifetimeGuard->load(std::memory_order_acquire)) {
                    m_changed.Publish();
                }
            });
            m_setterSubscriptions.emplace(setter.get(), subscription);
            setters.push_back(std::move(setter));
            SetValue(SettersProperty(), setters);
        }
        m_changed.Publish();
    }
}

std::size_t ResponsiveRule::AddChangedHandler(
    std::function<void()> handler)
{
    return m_changed.Subscribe(std::move(handler));
}

void ResponsiveRule::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed.Unsubscribe(handlerId);
}

void ResponsiveRule::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);
    if (&property == &SettersProperty()) {
        std::vector<std::shared_ptr<Styling::Triggers::Setter>> oldSetters;
        std::vector<std::shared_ptr<Styling::Triggers::Setter>> newSetters;
        if (oldValue.type() == typeid(std::vector<std::shared_ptr<Styling::Triggers::Setter>>)) {
            oldSetters = std::any_cast<
                std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(oldValue);
        }
        if (newValue.type() == typeid(std::vector<std::shared_ptr<Styling::Triggers::Setter>>)) {
            newSetters = std::any_cast<
                std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(newValue);
        }

        for (const auto& setter : oldSetters) {
            if (!setter ||
                std::find(newSetters.begin(), newSetters.end(), setter) !=
                    newSetters.end()) {
                continue;
            }
            const auto subscription = m_setterSubscriptions.find(setter.get());
            if (subscription != m_setterSubscriptions.end()) {
                setter->RemoveChangedHandler(subscription->second);
                m_setterSubscriptions.erase(subscription);
            }
        }

        for (const auto& setter : newSetters) {
            if (!setter ||
                m_setterSubscriptions.find(setter.get()) !=
                    m_setterSubscriptions.end()) {
                continue;
            }
            auto lifetimeGuard = m_lifetimeGuard;
            const auto subscription = setter->AddChangedHandler(
                [this, lifetimeGuard] {
                    if (lifetimeGuard->load(std::memory_order_acquire)) {
                        m_changed.Publish();
                    }
                });
            m_setterSubscriptions.emplace(setter.get(), subscription);
        }
    }
    m_changed.Publish();
}

bool ResponsiveRule::Matches(
    double availableWidth, double availableHeight) const
{
    const double minWidth = GetMinWidth();
    const double maxWidth = GetMaxWidth();
    const double minHeight = GetMinHeight();
    const double maxHeight = GetMaxHeight();
    if (std::isnan(minWidth) || std::isnan(maxWidth) ||
        std::isnan(minHeight) || std::isnan(maxHeight) ||
        maxWidth < minWidth || maxHeight < minHeight) {
        if (!m_invalidRangeReported) {
            NeoWPF::Diagnostics::LogError(
                "ResponsiveRule has an invalid minimum or maximum bound");
            m_invalidRangeReported = true;
        }
        return false;
    }

    m_invalidRangeReported = false;
    return availableWidth >= minWidth && availableWidth < maxWidth &&
        availableHeight >= minHeight && availableHeight < maxHeight;
}

}
