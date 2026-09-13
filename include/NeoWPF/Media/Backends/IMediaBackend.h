#pragma once

#include <string_view>

namespace NeoWPF::Media {

class IMediaBackend {
public:
    virtual ~IMediaBackend() = default;

    virtual std::string_view GetName() const noexcept = 0;
};

}
