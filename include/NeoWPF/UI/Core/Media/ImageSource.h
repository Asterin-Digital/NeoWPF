#pragma once

#include <memory>
#include <string>

namespace NeoWPF::UI::Core::Media {

class ImageSource {
public:
    virtual ~ImageSource() = default;

    virtual int GetPixelWidth() const noexcept = 0;
    virtual int GetPixelHeight() const noexcept = 0;
    virtual bool TryGetPixels(const void*& outPixels, unsigned int& outStride) const noexcept = 0;

    virtual std::wstring ToString() const { return L"ImageSource"; }
};

}
