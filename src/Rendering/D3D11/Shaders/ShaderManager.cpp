#include <NeoWPF/Rendering/ShaderSources.h>

namespace NeoWPF::Rendering::D3D11 {

namespace {

const char* GetShaderSourceForProgram(bool isText, bool isVertex)
{
    if (isText) {
        return isVertex ? GetTextVertexShaderSource() : GetTextPixelShaderSource();
    }
    return isVertex ? GetRectVertexShaderSource() : GetRectPixelShaderSource();
}

}

const char* GetShaderSourceForProgram(bool isText, bool isVertex);

}
