#pragma once

namespace NeoWPF::UI {

struct ContentBounds {
    int width = 0;
    int height = 0;
};

class IContentAware {
public:
    virtual ~IContentAware() = default;
    virtual ContentBounds GetContentBounds() const noexcept = 0;
};

}
