/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string_view>

namespace NeoWPF::Media {

class IMediaBackend {
public:
    virtual ~IMediaBackend() = default;

    virtual std::string_view GetName() const noexcept = 0;
};

}
