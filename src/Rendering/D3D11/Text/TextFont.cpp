#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <Windows.h>
#include <dwrite.h>
#include <wrl.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace NeoWPF::Rendering::Text {

class CustomFontCollectionLoader;
class FontFileEnumerator;

std::mutex g_customFontMutex;
std::vector<std::wstring> g_registeredFontFilePaths;
std::vector<ComPtr<IDWriteFontFile>> g_registeredFontFiles;
ComPtr<IDWriteFontCollection> g_customFontCollection;
ComPtr<IDWriteFontCollectionLoader> g_customFontCollectionLoader;
std::once_flag g_customFontCollectionLoaderOnce;

ComPtr<IDWriteFactory> GetDWriteFactory()
{
    static std::once_flag factoryOnce;
    static ComPtr<IDWriteFactory> factory;
    std::call_once(factoryOnce, []() {
        HRESULT hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(factory.GetAddressOf()));
        if (FAILED(hr)) {
            ::NeoWPF::Diagnostics::LogError("GetDWriteFactory: DWriteCreateFactory failed 0x%08X", static_cast<unsigned int>(hr));
            factory = nullptr;
        }
    });
    return factory;
}

class FontFileEnumerator : public IDWriteFontFileEnumerator {
public:
    FontFileEnumerator(IUnknown* loader, const std::vector<ComPtr<IDWriteFontFile>>& fontFiles)
        : loader(loader), fontFiles(fontFiles), index(-1)
    {
        if (loader) {
            loader->AddRef();
        }
    }

    ~FontFileEnumerator()
    {
        if (loader) {
            loader->Release();
        }
    }

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) {
            return E_POINTER;
        }
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteFontFileEnumerator)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() override
    {
        return static_cast<ULONG>(refCount.fetch_add(1, std::memory_order_relaxed) + 1);
    }

    STDMETHOD_(ULONG, Release)() override
    {
        const ULONG count = static_cast<ULONG>(refCount.fetch_sub(1, std::memory_order_acq_rel) - 1);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    STDMETHOD(MoveNext)(BOOL* hasCurrentFontFile) override
    {
        if (!hasCurrentFontFile) {
            return E_POINTER;
        }
        if (index + 1 >= static_cast<int>(fontFiles.size())) {
            *hasCurrentFontFile = FALSE;
            return S_OK;
        }
        index += 1;
        *hasCurrentFontFile = TRUE;
        return S_OK;
    }

    STDMETHOD(GetCurrentFontFile)(IDWriteFontFile** fontFile) override
    {
        if (!fontFile) {
            return E_POINTER;
        }
        if (index < 0 || index >= static_cast<int>(fontFiles.size())) {
            return E_FAIL;
        }
        return fontFiles[index].CopyTo(fontFile);
    }

    STDMETHOD(GetCurrentFontFileLoader)(IDWriteFontCollectionLoader** fontFileLoader)
    {
        if (!fontFileLoader) {
            return E_POINTER;
        }
        return loader->QueryInterface(__uuidof(IDWriteFontCollectionLoader), reinterpret_cast<void**>(fontFileLoader));
    }

private:
    IUnknown* loader;
    std::vector<ComPtr<IDWriteFontFile>> fontFiles;
    int index;
    std::atomic<ULONG> refCount{1};
};

class CustomFontCollectionLoader : public IDWriteFontCollectionLoader {
public:
    CustomFontCollectionLoader() = default;

    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
    {
        if (!ppvObject) {
            return E_POINTER;
        }
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteFontCollectionLoader)) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() override
    {
        return static_cast<ULONG>(refCount.fetch_add(1, std::memory_order_relaxed) + 1);
    }

    STDMETHOD_(ULONG, Release)() override
    {
        const ULONG count = static_cast<ULONG>(refCount.fetch_sub(1, std::memory_order_acq_rel) - 1);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    STDMETHOD(GetLoaderKeySize)(UINT32* keySize)
    {
        if (!keySize) {
            return E_POINTER;
        }
        *keySize = sizeof(UINT32);
        return S_OK;
    }

    STDMETHOD(CreateEnumeratorFromKey)(IDWriteFactory* , const void* , UINT32 , IDWriteFontFileEnumerator** fontFileEnumerator) override
    {
        if (!fontFileEnumerator) {
            return E_POINTER;
        }

        std::lock_guard<std::mutex> lock(g_customFontMutex);
        if (g_registeredFontFiles.empty()) {
            *fontFileEnumerator = nullptr;
            return E_FAIL;
        }

        auto enumerator = new FontFileEnumerator(this, g_registeredFontFiles);
        enumerator->AddRef();
        *fontFileEnumerator = enumerator;
        return S_OK;
    }

private:
    std::atomic<ULONG> refCount{1};
};

ComPtr<IDWriteFontCollectionLoader> GetCustomFontCollectionLoader()
{
    std::call_once(g_customFontCollectionLoaderOnce, []() {
        g_customFontCollectionLoader = ComPtr<IDWriteFontCollectionLoader>(new CustomFontCollectionLoader());
    });
    return g_customFontCollectionLoader;
}

IDWriteFontCollection* GetCustomFontCollection()
{
    {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        if (g_customFontCollection) {
            return g_customFontCollection.Get();
        }
        if (g_registeredFontFiles.empty()) {
            return nullptr;
        }
    }

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    if (!factory) {
        return nullptr;
    }

    ComPtr<IDWriteFontCollectionLoader> loader = GetCustomFontCollectionLoader();
    if (!loader) {
        return nullptr;
    }

    const UINT32 loaderKeySize = sizeof(UINT32);
    UINT32 loaderKey = 0;
    ComPtr<IDWriteFontCollection> createdCollection;
    HRESULT hr = factory->CreateCustomFontCollection(
        loader.Get(),
        &loaderKey,
        loaderKeySize,
        &createdCollection);
    if (FAILED(hr)) {
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        if (!g_customFontCollection) {
            g_customFontCollection = createdCollection;
        }
    }

    return g_customFontCollection.Get();
}

bool RegisterFontFile(const std::wstring& filePath)
{
    if (filePath.empty()) {
        return false;
    }

    ComPtr<IDWriteFactory> factory = GetDWriteFactory();
    if (!factory) {
        return false;
    }

    ComPtr<IDWriteFontFile> fontFile;
    HRESULT hr = factory->CreateFontFileReference(filePath.c_str(), nullptr, &fontFile);
    if (FAILED(hr)) {
        ::NeoWPF::Diagnostics::LogError("RegisterFontFile: CreateFontFileReference failed for '%ls' 0x%08X", filePath.c_str(), static_cast<unsigned int>(hr));
    
        return false;
    }

    BOOL isSupported = FALSE;
    DWRITE_FONT_FILE_TYPE fileType = DWRITE_FONT_FILE_TYPE_UNKNOWN;
    DWRITE_FONT_FACE_TYPE faceType = DWRITE_FONT_FACE_TYPE_UNKNOWN;
    UINT32 numberOfFaces = 0;
    hr = fontFile->Analyze(&isSupported, &fileType, &faceType, &numberOfFaces);
    if (FAILED(hr) || !isSupported || numberOfFaces == 0) {
        ::NeoWPF::Diagnostics::LogError("RegisterFontFile: Analyze failed for '%ls' 0x%08X", filePath.c_str(), static_cast<unsigned int>(hr));
        return false;
    }

    std::lock_guard<std::mutex> lock(g_customFontMutex);
    if (std::find(g_registeredFontFilePaths.begin(), g_registeredFontFilePaths.end(), filePath) != g_registeredFontFilePaths.end()) {
        return true;
    }

    g_registeredFontFilePaths.push_back(filePath);
    g_registeredFontFiles.push_back(fontFile);
    g_customFontCollection = nullptr;
    return true;
}

std::vector<std::wstring> GetRegisteredFontFiles()
{
    std::lock_guard<std::mutex> lock(g_customFontMutex);
    return g_registeredFontFilePaths;
}

}
