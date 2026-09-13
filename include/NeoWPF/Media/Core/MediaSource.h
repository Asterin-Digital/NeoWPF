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
