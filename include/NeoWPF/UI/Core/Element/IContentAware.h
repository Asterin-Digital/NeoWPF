/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

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
