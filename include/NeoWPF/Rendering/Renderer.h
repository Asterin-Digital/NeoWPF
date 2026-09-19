/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Rendering/Clipping.h>
#include <vector>
#include <cstdint>

namespace NeoWPF::Rendering {

/*
    @summary
    Holds the drawing commands for a single frame.

    @details
    This is the renderer's internal command list.
    It keeps track of rectangles, rounded rectangles, and textured quads that get
    executed when the frame is presented.

    @internal
*/
struct CommandList {
    enum class CommandKind {
        Rect,
        RoundedRect,
        TexturedQuad,
    };

    /*
        @summary
        Describes a filled rectangle draw command.

        @internal
    */
    struct DrawRect { double x; double y; double w; double h; uint32_t color; float opacity; ClipRect clip; float rotationRadians = 0.0f; double pivotX = 0.0; double pivotY = 0.0; };

    /*
        @summary
        Describes a filled rounded-rectangle draw command.

        @internal
    */
    struct DrawRoundedRect { double x; double y; double w; double h; double topLeft; double topRight; double bottomRight; double bottomLeft; uint32_t color; float opacity; ClipRect clip; float rotationRadians = 0.0f; double pivotX = 0.0; double pivotY = 0.0; };

    /*
        @summary
        Describes a textured quad draw command.

        @internal
    */
    struct DrawTexturedQuad { double x; double y; double w; double h; float u0; float v0; float u1; float v1; uint32_t textureId; uint32_t tintColor; float opacity; ClipRect clip; float rotationRadians = 0.0f; double pivotX = 0.0; double pivotY = 0.0; };

    struct Command {
        CommandKind kind;
        DrawRect rect{};
        DrawRoundedRect roundedRect{};
        DrawTexturedQuad texturedQuad{};
    };

    std::vector<Command> commands;
    std::uint32_t clearColor = 0xFFFFFFFFu;
    bool hasClearColor = false;

    /*
        @summary
        Clears out all queued drawing commands.

        @internal
    */
    void Clear() { commands.clear(); }

    /*
        @summary
        Adds a filled rectangle to the draw queue.

        @param x
        The left position of the rectangle.

        @param y
        The top position of the rectangle.

        @param w
        The rectangle width.

        @param h
        The rectangle height.

        @param color
        The fill color, encoded as an ARGB value.

        @param opacity
        The opacity applied to the draw.

        @param clip
        The clipping rectangle to constrain the draw.

        @param rotationRadians
        The optional rotation applied to the rectangle.

        @param pivotX
        The world-space X coordinate of the rotation pivot.

        @param pivotY
        The world-space Y coordinate of the rotation pivot.

        @internal
    */
    void DrawRect(double x, double y, double w, double h, uint32_t color, float opacity = 1.0f, const ClipRect& clip = ClipRect{}, float rotationRadians = 0.0f, double pivotX = 0.0, double pivotY = 0.0) {
        commands.push_back({CommandKind::Rect, {x, y, w, h, color, opacity, clip, rotationRadians, pivotX, pivotY}, {}, {}});
    }

    /*
        @summary
        Adds a filled rounded rectangle to the draw queue.

        @param x
        The left position of the rectangle.

        @param y
        The top position of the rectangle.

        @param w
        The rectangle width.

        @param h
        The rectangle height.

        @param topLeft
        The radius of the top-left corner.

        @param topRight
        The radius of the top-right corner.

        @param bottomRight
        The radius of the bottom-right corner.

        @param bottomLeft
        The radius of the bottom-left corner.

        @param color
        The fill color, encoded as an ARGB value.

        @param opacity
        The opacity applied to the draw.

        @param clip
        The clipping rectangle to constrain the draw.

        @param rotationRadians
        The optional rotation applied to the rectangle.

        @param pivotX
        The world-space X coordinate of the rotation pivot.

        @param pivotY
        The world-space Y coordinate of the rotation pivot.

        @internal
    */
    void DrawRoundedRect(double x, double y, double w, double h, double topLeft, double topRight, double bottomRight, double bottomLeft, uint32_t color, float opacity = 1.0f, const ClipRect& clip = ClipRect{}, float rotationRadians = 0.0f, double pivotX = 0.0, double pivotY = 0.0) {
        commands.push_back({CommandKind::RoundedRect, {}, {x, y, w, h, topLeft, topRight, bottomRight, bottomLeft, color, opacity, clip, rotationRadians, pivotX, pivotY}, {}});
    }

    /*
        @summary
        Adds a textured quad to the draw queue.

        @param x
        The left position of the quad.

        @param y
        The top position of the quad.

        @param w
        The quad width.

        @param h
        The quad height.

        @param u0
        The normalized U coordinate for the top-left corner.

        @param v0
        The normalized V coordinate for the top-left corner.

        @param u1
        The normalized U coordinate for the bottom-right corner.

        @param v1
        The normalized V coordinate for the bottom-right corner.

        @param tex
        The texture identifier.

        @param opacity
        The opacity applied to the draw.

        @param clip
        The clipping rectangle to constrain the draw.

       @note
       Always submit the full quad. The renderer takes care of clipping and UV remapping.

        @internal
    */
    void DrawTexturedQuad(double x, double y, double w, double h, float u0, float v0, float u1, float v1, uint32_t tex, uint32_t tintColor, float opacity = 1.0f, const ClipRect& clip = ClipRect{}, float rotationRadians = 0.0f, double pivotX = 0.0, double pivotY = 0.0) {
        commands.push_back({CommandKind::TexturedQuad, {}, {}, {x, y, w, h, u0, v0, u1, v1, tex, tintColor, opacity, clip, rotationRadians, pivotX, pivotY}});
    }
};

/*
    @summary
    Sets up the renderer for the target window.

    @param nativeWindowHandle
    The native window handle that owns the rendering surface.

    @internal
*/
void Renderer_Initialize(void* nativeWindowHandle);

/*
    @summary
    Releases the renderer's resources.

    @internal
*/
void Renderer_Shutdown();

/*
    @summary
    Resizes the renderer's output surface.

    @param w
    The new width of the render target.

    @param h
    The new height of the render target.

    @internal
*/
void Renderer_Resize(unsigned int w, unsigned int h);

/*
    @summary
    Starts a new frame for the renderer.

    @internal
*/
void Renderer_BeginFrame();

/*
    @summary
    Runs the queued draw commands.

    @param cmd
    The command list containing the draw operations to execute.

    @internal
*/
void Renderer_Execute(const CommandList& cmd);

/*
    @summary
    Pushes the current frame to the display.

    @internal
*/
void Renderer_Present();

/*
    @summary
    Creates a texture from raw RGBA data.

    @param w
    The texture width in pixels.

    @param h
    The texture height in pixels.

    @param rgbaData
    The source pixel data, in RGBA byte order.

    @returns
    A renderer-owned texture identifier.

    @internal
*/
uint32_t Renderer_CreateTexture(unsigned int w, unsigned int h, const void* rgbaData);

/*
    @summary
    Releases a renderer-owned texture.

    @param id
    The texture identifier to destroy.

    @internal
*/
void Renderer_DestroyTexture(uint32_t id);

/*
    @summary
    Clears all renderer-managed textures immediately.

    @internal
*/
void Renderer_ClearTextures();

/*
    @summary
    Returns true when the renderer and its device are initialized.

    @internal
*/
bool Renderer_IsInitialized();

}
