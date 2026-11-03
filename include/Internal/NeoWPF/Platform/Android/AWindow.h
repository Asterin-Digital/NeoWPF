/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <android/native_window.h>

#include <atomic>
#include <cstdint>

namespace NeoWPF::Platform::Android {

/*
    @summary
    Retains the Android native surface and its pixel dimensions.

    @internal
*/
class AWindow final {
public:
    ~AWindow();

    /*
        @summary
        Returns the currently retained native surface.

        @internal
    */
    ANativeWindow* NativeWindow() const noexcept;

    /*
        @summary
        Replaces the retained native surface and takes ownership of its reference.

        @param nativeWindow
        Acquired Android native window, or null.

        @internal
    */
    void SetNativeWindow(ANativeWindow* nativeWindow) noexcept;

    /*
        @summary
        Sets the current surface dimensions in pixels.

        @param width
        Surface width.

        @param height
        Surface height.

        @internal
    */
    void SetSize(unsigned int width, unsigned int height) noexcept;

    /*
        @summary
        Returns the current surface dimensions in pixels.

        @param width
        Receives the surface width.

        @param height
        Receives the surface height.

        @internal
    */
    void GetSize(unsigned int& width, unsigned int& height) const noexcept;

private:
    std::atomic<ANativeWindow*> m_nativeWindow{nullptr};
    std::atomic<std::uint64_t> m_size{0};
};

}