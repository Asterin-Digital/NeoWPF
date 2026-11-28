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

#include <NeoWPF/Contracts/IGeneral.h>
#include <NeoWPF/Contracts/IWindowChrome.h>
#include <NeoWPF/Contracts/IWindowing.h>

#include <memory>

namespace NeoWPF::Platform::Linux {

enum class LinuxDisplayBackend {
    Wayland,
    X11,
};

/*
    @summary
    Resolves a Linux display backend from environment values.

    @param overrideValue
    Explicit backend override.

    @param xdgSessionType
    Current desktop session type.

    @param waylandDisplay
    Wayland display environment value.

    @internal
*/
LinuxDisplayBackend ResolveLinuxDisplayBackend(
    const char* overrideValue,
    const char* xdgSessionType,
    const char* waylandDisplay) noexcept;

class LCompositor final {
public:
    LCompositor();
    ~LCompositor();

    LCompositor(const LCompositor&) = delete;
    LCompositor& operator=(const LCompositor&) = delete;

    /*
        @summary
        Reports whether the selected Linux display backend connected.

        @internal
    */
    bool IsConnected() const noexcept;
    NeoWPF::Contracts::IWindowing& Windowing() noexcept;
    NeoWPF::Contracts::IClipboard& Clipboard() noexcept;
    NeoWPF::Contracts::IWindowChrome* Chrome() noexcept;

private:
    bool m_connected = false;
    std::unique_ptr<NeoWPF::Contracts::IWindowing> m_windowing;
    std::unique_ptr<NeoWPF::Contracts::IClipboard> m_clipboard;
    std::unique_ptr<NeoWPF::Contracts::IWindowChrome> m_chrome;
};

}
