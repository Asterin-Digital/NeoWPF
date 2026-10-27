/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>

namespace NeoWPF::Input::Core {
class PointerEventArgs;
}

namespace NeoWPF::Contracts {

/*
    @summary
    Receives normalized pointer packets from platform input producers.

    @internal
*/
class IPointerInput {
public:
    virtual ~IPointerInput() = default;

    /*
        @summary
        Submits a pointer event for the currently selected framework input root.

        @param eventArgs
        The normalized pointer event packet.

        @internal
    */
    virtual void SubmitPointerEvent(
        const NeoWPF::Input::Core::PointerEventArgs& eventArgs) = 0;
};

}