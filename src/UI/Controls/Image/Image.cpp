#include <NeoWPF/UI/Controls/Image/Image.h>
#include <NeoWPF/UI/Core/Media/BitmapImage.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace {
    const NeoWPF::UI::Properties::DependencyProperty& EnsureSourceProperty()
    {
        NeoWPF::UI::Properties::RegisterImageProperties();
        if (const auto* property = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Source", typeid(NeoWPF::UI::Image))) {
            return *property;
        }
        return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Image, std::shared_ptr<NeoWPF::UI::Core::Media::ImageSource>>("Source");
    }

    const NeoWPF::UI::Properties::DependencyProperty& EnsureStretchProperty()
    {
        NeoWPF::UI::Properties::RegisterImageProperties();
        if (const auto* property = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("Stretch", typeid(NeoWPF::UI::Image))) {
            return *property;
        }
        return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Image, NeoWPF::UI::Core::Media::Stretch>("Stretch");
    }

    const NeoWPF::UI::Properties::DependencyProperty& EnsureStretchDirectionProperty()
    {
        NeoWPF::UI::Properties::RegisterImageProperties();
        if (const auto* property = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty("StretchDirection", typeid(NeoWPF::UI::Image))) {
            return *property;
        }
        return NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Image, NeoWPF::UI::Core::Media::StretchDirection>("StretchDirection");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::SourceProperty = EnsureSourceProperty();
const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::StretchProperty = EnsureStretchProperty();
const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Image::StretchDirectionProperty = EnsureStretchDirectionProperty();

namespace NeoWPF::UI {

namespace {
    void ComputeImageRenderSize(int sourceWidth,
                                int sourceHeight,
                                double targetWidth,
                                double targetHeight,
                                Core::Media::Stretch stretch,
                                Core::Media::StretchDirection direction,
                                double& outWidth,
                                double& outHeight)
    {
        outWidth = sourceWidth;
        outHeight = sourceHeight;

        if (sourceWidth <= 0 || sourceHeight <= 0 || targetWidth <= 0 || targetHeight <= 0) {
            return;
        }

        if (stretch == Core::Media::Stretch::Fill) {
            outWidth = targetWidth;
            outHeight = targetHeight;
            return;
        }

        if (stretch == Core::Media::Stretch::None) {
            return;
        }

        const double scaleX = targetWidth / static_cast<double>(sourceWidth);
        const double scaleY = targetHeight / static_cast<double>(sourceHeight);
        double scale = (stretch == Core::Media::Stretch::UniformToFill) ? (std::max)(scaleX, scaleY) : (std::min)(scaleX, scaleY);

        if (direction == Core::Media::StretchDirection::DownOnly && scale > 1.0f) {
            scale = 1.0f;
        } else if (direction == Core::Media::StretchDirection::UpOnly && scale < 1.0f) {
            scale = 1.0f;
        }

        outWidth = static_cast<double>(sourceWidth) * scale;
        outHeight = static_cast<double>(sourceHeight) * scale;

        if (outWidth <= 0) {
            outWidth = 1;
        }
        if (outHeight <= 0) {
            outHeight = 1;
        }
    }
}

Image::Image()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

Image::~Image()
{
    DestroyTexture();
}

void Image::SetSource(const std::shared_ptr<Core::Media::ImageSource>& source)
{
    if (IsApplyingPropertyValue()) {
        if (m_source == source) {
            return;
        }

        m_source = source;
        m_sourceDirty = true;
        SetupAsyncLoading();
        InvalidateMeasure();
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Source", *this);
    if (property) {
        SetValue(*property, source);
        return;
    }

    if (m_source == source) {
        return;
    }

    m_source = source;
    m_sourceDirty = true;
    SetupAsyncLoading();
    InvalidateMeasure();
    InvalidateVisual();
}

void Image::SetStretch(Core::Media::Stretch stretch) noexcept
{
    if (IsApplyingPropertyValue()) {
        if (m_stretch == stretch) {
            return;
        }

        m_stretch = stretch;
        InvalidateArrange();
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Stretch", *this);
    if (property) {
        SetValue(*property, stretch);
        return;
    }

    if (m_stretch == stretch) {
        return;
    }

    m_stretch = stretch;
    InvalidateArrange();
    InvalidateVisual();
}

void Image::SetStretchDirection(Core::Media::StretchDirection direction) noexcept
{
    if (IsApplyingPropertyValue()) {
        if (m_stretchDirection == direction) {
            return;
        }

        m_stretchDirection = direction;
        InvalidateArrange();
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("StretchDirection", *this);
    if (property) {
        SetValue(*property, direction);
        return;
    }

    if (m_stretchDirection == direction) {
        return;
    }

    m_stretchDirection = direction;
    InvalidateArrange();
    InvalidateVisual();
}

void Image::Measure(double availableWidth, double availableHeight)
{
    UpdateSourceState();

    if (m_pixelWidth <= 0 || m_pixelHeight <= 0) {
        SetDesiredSize(0, 0);
        return;
    }

    SetDesiredSize(m_pixelWidth, m_pixelHeight);
}

void Image::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    UpdateSourceState();
    EnsureTexture();

    m_lastArrangeWidth = finalWidth;
    m_lastArrangeHeight = finalHeight;
}

void Image::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_textureId == 0 || GetFinalWidth() <= 0 || GetFinalHeight() <= 0) {
        return;
    }

    double drawWidth = 0.0;
    double drawHeight = 0.0;
    ComputeImageRenderSize(m_pixelWidth, m_pixelHeight, GetFinalWidth(), GetFinalHeight(), m_stretch, m_stretchDirection, drawWidth, drawHeight);

    if (drawWidth <= 0 || drawHeight <= 0) {
        return;
    }

    const auto clip = GetClipRect();
    const auto renderTransform = GetWorldRenderTransform();
    double drawX = (std::max)(0.0, (GetFinalWidth() - drawWidth) / 2.0);
    double drawY = (std::max)(0.0, (GetFinalHeight() - drawHeight) / 2.0);
    double drawWidthF = drawWidth;
    double drawHeightF = drawHeight;
    renderTransform.positionMatrix.TransformPoint(drawX, drawY, drawX, drawY);
    double right = drawX + drawWidthF;
    double bottom = drawY + drawHeightF;
    renderTransform.positionMatrix.TransformPoint(right, bottom, right, bottom);
    drawWidthF = std::abs(right - drawX);
    drawHeightF = std::abs(bottom - drawY);

    commands.DrawTexturedQuad(
        drawX,
        drawY,
        drawWidthF,
        drawHeightF,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        m_textureId,
        0xFFFFFFFFu,
        GetOpacity(),
        clip,
        renderTransform.rotationRadians,
        renderTransform.pivot.X,
        renderTransform.pivot.Y);
}

void Image::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                       const Properties::PropertyMetadata& metadata,
                                       const std::any& oldValue,
                                       const std::any& newValue,
                                       Properties::PropertyValueSource source)
{
    FrameworkElement::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);

    const std::string propertyName = property.GetName();
    if (propertyName == "Source") {
        m_sourceDirty = true;
        InvalidateMeasure();
        InvalidateVisual();
        return;
    }

    if (propertyName == "Stretch" || propertyName == "StretchDirection") {
        InvalidateArrange();
        InvalidateVisual();
    }
}

void Image::UpdateSourceState()
{
    if (!m_sourceDirty) {
        return;
    }

    m_pixelWidth = 0;
    m_pixelHeight = 0;
    m_pixelData = nullptr;
    m_pixelStride = 0;
    m_sourceDirty = false;

    if (!m_source) {
        DestroyTexture();
        return;
    }

    m_pixelWidth = m_source->GetPixelWidth();
    m_pixelHeight = m_source->GetPixelHeight();

    const void* pixels = nullptr;
    unsigned int stride = 0;
    if (m_source->TryGetPixels(pixels, stride) && pixels && m_pixelWidth > 0 && m_pixelHeight > 0) {
        m_pixelData = pixels;
        m_pixelStride = stride;
        DestroyTexture();
    } else {
        DestroyTexture();
        m_pixelWidth = 0;
        m_pixelHeight = 0;
    }
}

void Image::EnsureTexture()
{
    if (m_pixelData == nullptr || m_pixelWidth <= 0 || m_pixelHeight <= 0) {
        return;
    }

    if (m_textureId != 0) {
        return;
    }

    m_textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(m_pixelWidth), static_cast<unsigned int>(m_pixelHeight), m_pixelData);
    if (m_textureId == 0) {
        NeoWPF::Diagnostics::LogError("Image: failed to create texture from source pixels");
    }
}

void Image::DestroyTexture() noexcept
{
    if (m_textureId != 0 && NeoWPF::Rendering::Renderer_IsInitialized()) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
    }
    m_textureId = 0;
}

void Image::SetupAsyncLoading()
{
    auto bitmapImage = std::dynamic_pointer_cast<Core::Media::BitmapImage>(m_source);
    if (!bitmapImage) {
        return;
    }

    bitmapImage->SetDownloadCompletedCallback([this]() {
        m_sourceDirty = true;
        InvalidateVisual();
    });
}

}
