#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/CallStackTracer.h>
#include <algorithm>
#include <typeinfo>

namespace NeoWPF::UI {

namespace {

void RefreshBindingsRecursively(Visual* node) {
    if (!node) {
        return;
    }

    if (auto depObj = dynamic_cast<Properties::DependencyObject*>(node)) {
        depObj->RefreshBindingExpressions();
    }

    for (auto child : node->GetVisualChildren()) {
        RefreshBindingsRecursively(child);
    }
}

}

bool FrameworkElement::s_frameworkElementLogging = false;

std::any FrameworkElement::GetDataContext() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("DataContext", *this);
    if (property) {
        const auto value = GetRawValue(*property);
        if (value.has_value()) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
                "[GetDataContext] Found DataContext via property on %p, type=%s", this, value.type().name());
            return value;
        }
    }
    
    if (m_dataContext.has_value()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
            "[GetDataContext] Found local m_dataContext on %p, type=%s", this, m_dataContext.type().name());
        return m_dataContext;
    }
    
    if (const auto* parent = GetInheritanceParent()) {
        if (auto parentFE = dynamic_cast<const FrameworkElement*>(parent)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
                "[GetDataContext] Walking to parent %p from %p", parent, this);
            return parentFE->GetDataContext();
        }
    }
    
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
        "[GetDataContext] No DataContext found on %p, returning empty", this);
    return std::any();
}

void FrameworkElement::SetDataContext(const std::any& dataContext) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_dataContext = dataContext;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("DataContext", *this);
    if (property) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
            "[SetDataContext] Setting DataContext via DependencyProperty on %p, refreshing bindings", this);
        SetValue(*property, dataContext);

        RefreshBindingsRecursively(this);
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, 
        "[SetDataContext] Setting local DataContext on %p, refreshing bindings", this);
    m_dataContext = dataContext;

    RefreshBindingsRecursively(this);
}

LayoutSizeMode FrameworkElement::GetWidthMode() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("WidthMode", *this);
     if (property) {
        try {
            return GetValue<LayoutSizeMode>(*property);
        } catch (const std::bad_any_cast&) {
            return m_widthMode;
        }
    }

    return m_widthMode;
}

LayoutSizeMode FrameworkElement::GetHeightMode() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HeightMode", *this);
    if (property) {
        try {
            return GetValue<LayoutSizeMode>(*property);
        } catch (const std::bad_any_cast&) {
            return m_heightMode;
        }
    }

    return m_heightMode;
}

int FrameworkElement::GetWidth() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Width", *this);
    if (property) {
        try {
            return GetValue<int>(*property);
        } catch (const std::bad_any_cast&) {
            return m_width;
        }
    }

    return m_width;
}

void FrameworkElement::SetWidth(int width) noexcept
{
    const int clampedWidth = width < 0 ? 0 : width;
    if (IsApplyingPropertyValue()) {
        m_width = clampedWidth;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Width", *this);
    if (property) {
        SetValue(*property, clampedWidth);
        return;
    }

    m_width = clampedWidth;
    InvalidateVisual();
}

int FrameworkElement::GetHeight() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Height", *this);
    if (property) {
        try {
            return GetValue<int>(*property);
        } catch (const std::bad_any_cast&) {
            return m_height;
        }
    }

    return m_height;
}

Thickness FrameworkElement::GetMargin() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Margin", *this);
    if (property) {
        try {
            return GetValue<Thickness>(*property);
        } catch (const std::bad_any_cast&) {
            return m_margin;
        }
    }

    return m_margin;
}

void FrameworkElement::SetMargin(Thickness margin) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_margin = margin;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Margin", *this);
    if (property) {
        if (GetMargin() != margin) {
            SetValue(*property, margin);
        }
        return;
    }

    if (m_margin != margin) {
        m_margin = margin;
        InvalidateMeasure();
    }
}

void FrameworkElement::SetMargin(int uniform) noexcept
{
    SetMargin(Thickness(uniform));
}

void FrameworkElement::SetMargin(int left, int top, int right, int bottom) noexcept
{
    SetMargin(Thickness(left, top, right, bottom));
}

int FrameworkElement::GetMarginLeft() const noexcept
{
    return m_margin.Left;
}

int FrameworkElement::GetMarginTop() const noexcept
{
    return m_margin.Top;
}

int FrameworkElement::GetMarginRight() const noexcept
{
    return m_margin.Right;
}

int FrameworkElement::GetMarginBottom() const noexcept
{
    return m_margin.Bottom;
}

int FrameworkElement::GetMarginHorizontal() const noexcept
{
    return m_margin.Left + m_margin.Right;
}

int FrameworkElement::GetMarginVertical() const noexcept
{
    return m_margin.Top + m_margin.Bottom;
}

Spacing FrameworkElement::GetSpacing() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Spacing", *this);
    if (property) {
        try {
            return GetValue<Spacing>(*property);
        } catch (const std::bad_any_cast&) {
            return m_spacing;
        }
    }

    return m_spacing;
}

void FrameworkElement::SetSpacing(Spacing spacing) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_spacing = spacing;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Spacing", *this);
    if (property) {
        if (GetSpacing() != spacing) {
            SetValue(*property, spacing);
        }
        return;
    }

    if (m_spacing != spacing) {
        m_spacing = spacing;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }
}

void FrameworkElement::SetSpacing(int uniform) noexcept
{
    SetSpacing(Spacing(uniform));
}

int FrameworkElement::GetHorizontalSpacing() const noexcept
{
    return m_spacing.Horizontal;
}

void FrameworkElement::SetHorizontalSpacing(int spacing) noexcept
{
    if (m_spacing.Horizontal != spacing) {
        m_spacing.Horizontal = spacing;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }
}

int FrameworkElement::GetVerticalSpacing() const noexcept
{
    return m_spacing.Vertical;
}

void FrameworkElement::SetVerticalSpacing(int spacing) noexcept
{
    if (m_spacing.Vertical != spacing) {
        m_spacing.Vertical = spacing;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }
}

void FrameworkElement::SetHeight(int height) noexcept
{
    const int clampedHeight = height < 0 ? 0 : height;
    if (IsApplyingPropertyValue()) {
        m_height = clampedHeight;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Height", *this);
    if (property) {
        SetValue(*property, clampedHeight);
        return;
    }

    m_height = clampedHeight;
    InvalidateVisual();
}

bool FrameworkElement::IsClipToBounds() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("ClipToBounds", *this);
    if (property) {
        try {
            return GetValue<bool>(*property);
        } catch (const std::bad_any_cast&) {
            return m_clipToBounds;
        }
    }

    return m_clipToBounds;
}

void FrameworkElement::SetClipToBounds(bool clipToBounds) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_clipToBounds = clipToBounds;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("ClipToBounds", *this);
    if (property) {
        SetValue(*property, clipToBounds);
        return;
    }

    if (m_clipToBounds != clipToBounds) {
        m_clipToBounds = clipToBounds;
        InvalidateVisual();
    }
}

void FrameworkElement::SetWidthMode(LayoutSizeMode mode) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_widthMode = mode;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("WidthMode", *this);
    if (property) {
        SetValue(*property, mode);
        return;
    }

    m_widthMode = mode;
    InvalidateVisual();
}

void FrameworkElement::SetHeightMode(LayoutSizeMode mode) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_heightMode = mode;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HeightMode", *this);
    if (property) {
        SetValue(*property, mode);
        return;
    }

    m_heightMode = mode;
    InvalidateVisual();
}

NeoWPF::UI::CornerRadius FrameworkElement::GetCornerRadius() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CornerRadius", *this);
    if (property) {
        try {
            return GetValue<NeoWPF::UI::CornerRadius>(*property);
        } catch (const std::bad_any_cast&) {
            return m_cornerRadius;
        }
    }

    return m_cornerRadius;
}

void FrameworkElement::SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_cornerRadius = cornerRadius;
        NeoWPF::Diagnostics::LogInfo("[FrameworkElement] SetCornerRadius applying=%f,%f,%f,%f", cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CornerRadius", *this);
    if (property) {
        NeoWPF::Diagnostics::LogInfo("[FrameworkElement] SetCornerRadius via property value=%f,%f,%f,%f", cornerRadius.TopLeft, cornerRadius.TopRight, cornerRadius.BottomRight, cornerRadius.BottomLeft);
        SetValue(*property, cornerRadius);
        return;
    }

    if (m_cornerRadius != cornerRadius) {
        m_cornerRadius = cornerRadius;
        InvalidateVisual();
    }
}

HorizontalAlignment FrameworkElement::GetHorizontalAlignment() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalAlignment", *this);
    if (property) {
        try {
            return GetValue<HorizontalAlignment>(*property);
        } catch (const std::bad_any_cast&) {
            return m_horizontalAlignment;
        }
    }

    return m_horizontalAlignment;
}

void FrameworkElement::SetHorizontalAlignment(HorizontalAlignment align) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_horizontalAlignment = align;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("HorizontalAlignment", *this);
    if (property) {
        SetValue(*property, align);
        return;
    }

    m_horizontalAlignment = align;
}

VerticalAlignment FrameworkElement::GetVerticalAlignment() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("VerticalAlignment", *this);
    if (property) {
        try {
            return GetValue<VerticalAlignment>(*property);
        } catch (const std::bad_any_cast&) {
            return m_verticalAlignment;
        }
    }

    return m_verticalAlignment;
}

void FrameworkElement::SetVerticalAlignment(VerticalAlignment align) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_verticalAlignment = align;
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("VerticalAlignment", *this);
    if (property) {
        SetValue(*property, align);
        return;
    }

    m_verticalAlignment = align;
}

int FrameworkElement::ResolveWidth(int availableWidth, int desiredWidth) const
{
    if (GetWidthMode() == LayoutSizeMode::Stretch) {
        return (std::max)(0, availableWidth);
    }

    if (!GetParent() && availableWidth > 0) {
        return (std::max)(0, availableWidth);
    }

    const int preferredWidth = GetWidth() > 0 ? GetWidth() : desiredWidth;
    return (std::max)(0, preferredWidth);
}

int FrameworkElement::ResolveHeight(int availableHeight, int desiredHeight) const
{
    if (GetHeightMode() == LayoutSizeMode::Stretch) {
        return (std::max)(0, availableHeight);
    }

    if (!GetParent() && availableHeight > 0) {
        return (std::max)(0, availableHeight);
    }

    const int preferredHeight = GetHeight() > 0 ? GetHeight() : desiredHeight;
    return (std::max)(0, preferredHeight);
}

void FrameworkElement::BeginAnimation(int fromWidth, int toWidth, int fromHeight, int toHeight, float durationSeconds)
{
    this->BeginAnimation("Width", fromWidth, toWidth, durationSeconds);
    this->BeginAnimation("Height", fromHeight, toHeight, durationSeconds);
}

void FrameworkElement::BeginAnimation(HorizontalAlignment horizontal, VerticalAlignment vertical, float durationSeconds)
{
    this->BeginAnimation("HorizontalAlignment", GetHorizontalAlignment(), horizontal, durationSeconds);
    this->BeginAnimation("VerticalAlignment", GetVerticalAlignment(), vertical, durationSeconds);
}

void FrameworkElement::Measure(int availableWidth, int availableHeight)
{
    if (s_frameworkElementLogging) {
        NeoWPF::Diagnostics::LogInfo(
            "[FrameworkElement::Measure] Type=%s, Available=(%d x %d)",
            typeid(*this).name(),
            availableWidth,
            availableHeight
        );
    }
    m_desiredWidth = availableWidth;
    m_desiredHeight = availableHeight;
}

void FrameworkElement::Arrange(int finalWidth, int finalHeight)
{
    if (s_frameworkElementLogging) {
        NeoWPF::Diagnostics::LogInfo(
            "[FrameworkElement::Arrange] Type=%s, Final=(%d x %d)",
            typeid(*this).name(),
            finalWidth,
            finalHeight
        );

        NeoWPF::Diagnostics::CallStackEntry stack[4];
        int count = NeoWPF::Diagnostics::CaptureCallStack(stack, 4, 1);
        for (int i = 0; i < count; ++i) {
            if (stack[i].symbol[0] != '\0') {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] %s @ %p", i, stack[i].symbol, stack[i].address);
            } else {
                NeoWPF::Diagnostics::LogInfo("  caller[%d] <unknown> @ %p", i, stack[i].address);
            }
        }
    }

    const bool stretchWidth = GetHorizontalAlignment() == HorizontalAlignment::Stretch;
    const bool stretchHeight = GetVerticalAlignment() == VerticalAlignment::Stretch;
    const bool hasExplicitWidth = GetWidth() > 0;
    const bool hasExplicitHeight = GetHeight() > 0;

    if (stretchWidth && !hasExplicitWidth && finalWidth >= 0) {
        m_finalWidth = std::max(0, finalWidth);
    } else {
        m_finalWidth = ResolveWidth(finalWidth, m_desiredWidth);
    }

    if (stretchHeight && !hasExplicitHeight && finalHeight >= 0) {
        m_finalHeight = std::max(0, finalHeight);
    } else {
        m_finalHeight = ResolveHeight(finalHeight, m_desiredHeight);
    }
}

const Properties::DependencyObject* FrameworkElement::GetInheritanceParent() const noexcept
{
    return dynamic_cast<const Properties::DependencyObject*>(GetParent());
}

void FrameworkElement::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                             const Properties::PropertyMetadata& metadata,
                                             const std::any& oldValue,
                                             const std::any& newValue,
                                             Properties::PropertyValueSource source)
{
    UIElement::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);

    if (property.GetName() == "DataContext") {
        RefreshBindingsRecursively(this);
    }
}

}
