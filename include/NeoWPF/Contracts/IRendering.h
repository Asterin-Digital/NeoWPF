/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <NeoWPF/Rendering/RenderSurface.h>

namespace NeoWPF::Rendering {
struct CommandList;
}

namespace NeoWPF::Contracts {

/*
    @summary
    Defines the rendering operations provided by a platform backend.
*/
class IRendering {
public:
    virtual ~IRendering() = default;

    /*
        @summary
        Initializes renderer resources that outlive individual surfaces.

        @param surface
        A surface used to select a compatible rendering device.

        @returns
        True when device resources are ready.

        @internal
    */
    virtual bool InitializeDevice(const NeoWPF::Rendering::RenderSurface& surface) = 0;

    /*
        @summary
        Creates an additional surface on the initialized renderer.

        @param surface
        The rendering surface to create.

        @returns
        True when the surface is ready for rendering.

        @internal
    */
    virtual bool CreateSurface(const NeoWPF::Rendering::RenderSurface& surface) = 0;

    /*
        @summary
        Releases the resources owned by a rendering surface.

        @param surface
        The rendering surface to release.

        @internal
    */
    virtual void DestroySurface(const NeoWPF::Rendering::RenderSurface& surface) noexcept = 0;
    /*
        @summary
        Releases renderer device resources and any remaining surfaces.

        @internal
    */
    virtual void ShutdownDevice() noexcept = 0;
    /*
        @summary
        Reports whether the rendering device is initialized.
    */
    virtual bool IsInitialized() const noexcept = 0;
    /*
        @summary
        Resizes a rendering surface.

        @param surface
        Surface to resize.

        @param width
        New surface width.

        @param height
        New surface height.
    */
    virtual void Resize(const NeoWPF::Rendering::RenderSurface& surface,
                        unsigned int width, unsigned int height) noexcept = 0;
    /*
        @summary
        Begins rendering a frame to a surface.
    */
    virtual void BeginFrame(const NeoWPF::Rendering::RenderSurface& surface) = 0;
    /*
        @summary
        Executes a command list against a rendering surface.

        @param surface
        Target surface.

        @param commands
        Commands to execute.
    */
    virtual void Execute(const NeoWPF::Rendering::RenderSurface& surface,
                         const NeoWPF::Rendering::CommandList& commands) = 0;
    /*
        @summary
        Presents the completed frame for a surface.
    */
    virtual void Present(const NeoWPF::Rendering::RenderSurface& surface) = 0;
    /*
        @summary
        Creates a texture from RGBA pixel data.

        @param width
        Texture width.

        @param height
        Texture height.

        @param rgbaData
        RGBA pixel buffer.
    */
    virtual std::uint32_t CreateTexture(unsigned int width, unsigned int height, const void* rgbaData) = 0;
    /*
        @summary
        Releases a texture by identifier.

        @param id
        Texture identifier.
    */
    virtual void DestroyTexture(std::uint32_t id) noexcept = 0;
    /*
        @summary
        Releases all textures owned by the renderer.
    */
    virtual void ClearTextures() noexcept = 0;
};

}
