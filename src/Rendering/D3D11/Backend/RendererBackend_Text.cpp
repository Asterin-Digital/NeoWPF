#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/ShaderSources.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <mutex>

using Microsoft::WRL::ComPtr;
using namespace NeoWPF::Rendering;

namespace NeoWPF::Rendering {
namespace D3D11 {
    extern ComPtr<ID3D11Device> g_device;
    extern ComPtr<ID3D11DeviceContext> g_context;
    extern ComPtr<ID3D11Buffer> g_constantBuffer;

    static std::unordered_map<uint32_t, ComPtr<ID3D11Texture2D>> g_textures;
    static std::unordered_map<uint32_t, ComPtr<ID3D11ShaderResourceView>> g_textureViews;
    static std::mutex g_textureMutex;
    static uint32_t g_nextTextureId = 1;

    uint32_t CreateTexture(unsigned int w, unsigned int h, const void* rgbaData)
    {
        std::lock_guard<std::mutex> lock(g_textureMutex);
        if (!g_device || w == 0 || h == 0 || !rgbaData) {
            return 0;
        }

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = rgbaData;
        initData.SysMemPitch = w * 4;

        ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = g_device->CreateTexture2D(&desc, &initData, &texture);
        if (FAILED(hr) || !texture) {
            ::NeoWPF::Diagnostics::LogError("CreateTexture failed with HRESULT 0x%08X", static_cast<unsigned int>(hr));
            return 0;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        ComPtr<ID3D11ShaderResourceView> srv;
        hr = g_device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv);
        if (FAILED(hr) || !srv) {
            ::NeoWPF::Diagnostics::LogError("CreateShaderResourceView failed with HRESULT 0x%08X", static_cast<unsigned int>(hr));
            return 0;
        }

        uint32_t id = g_nextTextureId++;
        if (id == 0) {
            id = g_nextTextureId++;
        }

        g_textures[id] = std::move(texture);
        g_textureViews[id] = std::move(srv);
        return id;
    }

    void DestroyTexture(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(g_textureMutex);
        g_textureViews.erase(id);
        g_textures.erase(id);
    }

    ComPtr<ID3D11ShaderResourceView> GetTextureShaderResourceView(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(g_textureMutex);
        auto it = g_textureViews.find(id);
        return it == g_textureViews.end() ? nullptr : it->second;
    }

}
}
