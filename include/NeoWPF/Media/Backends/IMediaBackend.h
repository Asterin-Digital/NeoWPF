/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <string_view>

namespace NeoWPF::Media {

class IMediaBackend {
public:
    virtual ~IMediaBackend() = default;

    virtual std::string_view GetName() const noexcept = 0;

    template<typename T>
    T* TryGetNativeBackend() noexcept
    {
        return dynamic_cast<T*>(this);
    }

    template<typename T>
    const T* TryGetNativeBackend() const noexcept
    {
        return dynamic_cast<const T*>(this);
    }
};

}
