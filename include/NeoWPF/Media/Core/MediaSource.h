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

#include <string>
#include <utility>

namespace NeoWPF::Media {

class MediaSource final {
public:
    MediaSource() = default;

    explicit MediaSource(std::string uri)
        : uri_(std::move(uri))
    {
    }

    const std::string& GetUri() const noexcept
    {
        return uri_;
    }

    bool IsEmpty() const noexcept
    {
        return uri_.empty();
    }

private:
    std::string uri_;
};

}
