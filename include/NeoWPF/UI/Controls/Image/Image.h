/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/ImageSource.h>
#include <NeoWPF/UI/Core/Media/Stretch.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI::Core::Media {
class BitmapImage;
}

namespace NeoWPF::UI {

class Image : public FrameworkElement {
public:
    static const Properties::DependencyProperty& SourceProperty;
    static const Properties::DependencyProperty& StretchProperty;
    static const Properties::DependencyProperty& StretchDirectionProperty;

    Image();
    ~Image();

    void SetSource(const std::shared_ptr<Core::Media::ImageSource>& source);
    std::shared_ptr<Core::Media::ImageSource> GetSource() const noexcept { return m_source; }

    void SetStretch(Core::Media::Stretch stretch) noexcept;
    Core::Media::Stretch GetStretch() const noexcept { return m_stretch; }

    void SetStretchDirection(Core::Media::StretchDirection direction) noexcept;
    Core::Media::StretchDirection GetStretchDirection() const noexcept { return m_stretchDirection; }

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;

private:
    std::shared_ptr<Core::Media::ImageSource> m_source;
    Core::Media::Stretch m_stretch = Core::Media::Stretch::Uniform;
    Core::Media::StretchDirection m_stretchDirection = Core::Media::StretchDirection::Both;
    uint32_t m_textureId = 0;
    double m_lastArrangeWidth = 0.0;
    double m_lastArrangeHeight = 0.0;
    int m_pixelWidth = 0;
    int m_pixelHeight = 0;
    const void* m_pixelData = nullptr;
    unsigned int m_pixelStride = 0;
    bool m_sourceDirty = false;

    void UpdateSourceState();
    void EnsureTexture();
    void DestroyTexture() noexcept;
    void SetupAsyncLoading();
};

}
