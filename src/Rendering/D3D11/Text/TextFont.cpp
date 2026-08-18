#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <Windows.h>
#include <dwrite.h>
#include <wrl.h>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <memory>

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

struct MemoryFont {
    std::shared_ptr<std::vector<unsigned char>> data;
    std::wstring familyName;
    uint64_t key = 0;
};

std::vector<MemoryFont> g_memoryFonts;
std::atomic<uint64_t> g_nextMemoryFontKey{1};

class MemoryFontFileStream : public IDWriteFontFileStream {
public:
    explicit MemoryFontFileStream(std::shared_ptr<std::vector<unsigned char>> data)
        : m_data(std::move(data)) {}

    STDMETHOD(QueryInterface)(REFIID riid, void** object) override {
        if (!object) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteFontFileStream)) {
            *object = this;
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHOD_(ULONG, AddRef)() override { return ++m_refCount; }
    STDMETHOD_(ULONG, Release)() override {
        const auto count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }
    STDMETHOD(ReadFileFragment)(const void** fragmentStart, UINT64 fileOffset, UINT64 fragmentSize, void** fragmentContext) override {
        if (!fragmentStart || !fragmentContext || fileOffset > m_data->size() || fragmentSize > m_data->size() - fileOffset) return E_FAIL;
        *fragmentStart = m_data->data() + fileOffset;
        *fragmentContext = nullptr;
        return S_OK;
    }
    STDMETHOD_(void, ReleaseFileFragment)(void*) override {}
    STDMETHOD(GetFileSize)(UINT64* fileSize) override {
        if (!fileSize) return E_POINTER;
        *fileSize = m_data->size();
        return S_OK;
    }
    STDMETHOD(GetLastWriteTime)(UINT64* lastWriteTime) override {
        if (!lastWriteTime) return E_POINTER;
        *lastWriteTime = 0;
        return S_OK;
    }

private:
    std::shared_ptr<std::vector<unsigned char>> m_data;
    ULONG m_refCount = 1;
};

class MemoryFontFileLoader : public IDWriteFontFileLoader {
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** object) override {
        if (!object) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IDWriteFontFileLoader)) {
            *object = this;
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHOD_(ULONG, AddRef)() override { return ++m_refCount; }
    STDMETHOD_(ULONG, Release)() override {
        const auto count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }
    STDMETHOD(CreateStreamFromKey)(const void* fontFileReferenceKey, UINT32 fontFileReferenceKeySize, IDWriteFontFileStream** fontFileStream) override {
        if (!fontFileStream) return E_POINTER;
        *fontFileStream = nullptr;
        if (!fontFileReferenceKey || fontFileReferenceKeySize != sizeof(uint64_t)) return E_INVALIDARG;
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        for (const auto& font : g_memoryFonts) {
            uint64_t key = 0;
            std::memcpy(&key, fontFileReferenceKey, sizeof(key));
            if (font.key == key) {
                *fontFileStream = new MemoryFontFileStream(font.data);
                return S_OK;
            }
        }
        return E_FAIL;
    }

private:
    ULONG m_refCount = 1;
};

ComPtr<IDWriteFontFileLoader> g_memoryFontLoader;
bool g_memoryFontLoaderRegistered = false;

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
    NEO_WPF_STARTUP_ACTION("RegisterFontFile");
    if (filePath.empty()) {
        return false;
    }

    ::NeoWPF::Diagnostics::Log(::NeoWPF::Diagnostics::Category::Application, ::NeoWPF::Diagnostics::Level::Info,
        "RegisterFontFile: registering font path '%ls'", filePath.c_str());

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

bool RegisterFontMemory(const unsigned char* data, size_t size, const std::wstring& familyName)
{
    if (!data || size == 0 || familyName.empty()) {
        ::NeoWPF::Diagnostics::LogError("RegisterFontMemory: invalid data, size, or family name");
        return false;
    }

    ::NeoWPF::Diagnostics::Log(
        ::NeoWPF::Diagnostics::Category::TextRendering,
        ::NeoWPF::Diagnostics::Level::Info,
        "RegisterFontMemory: validating '%ls' (%zu bytes)",
        familyName.c_str(), size);

    auto fontData = std::make_shared<std::vector<unsigned char>>(data, data + size);
    auto factory = GetDWriteFactory();
    if (!factory) {
        ::NeoWPF::Diagnostics::LogError("RegisterFontMemory: DirectWrite factory unavailable");
        return false;
    }

    const uint64_t memoryFontKey = g_nextMemoryFontKey.fetch_add(1);
    {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        if (!g_memoryFontLoader) {
            g_memoryFontLoader = ComPtr<IDWriteFontFileLoader>(new MemoryFontFileLoader());
        }

        if (!g_memoryFontLoaderRegistered) {
            const HRESULT loaderResult = factory->RegisterFontFileLoader(g_memoryFontLoader.Get());
            if (FAILED(loaderResult)) {
                ::NeoWPF::Diagnostics::LogError(
                    "RegisterFontMemory: RegisterFontFileLoader failed 0x%08X",
                    static_cast<unsigned int>(loaderResult));
                return false;
            }
            g_memoryFontLoaderRegistered = true;
        }

        g_memoryFonts.push_back({fontData, familyName, memoryFontKey});
    }

    ComPtr<IDWriteFontFile> fontFile;
    const HRESULT hr = factory->CreateCustomFontFileReference(
        &memoryFontKey, sizeof(memoryFontKey), g_memoryFontLoader.Get(), &fontFile);
    if (FAILED(hr) || !fontFile) {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        g_memoryFonts.erase(
            std::remove_if(g_memoryFonts.begin(), g_memoryFonts.end(),
                [memoryFontKey](const MemoryFont& font) { return font.key == memoryFontKey; }),
            g_memoryFonts.end());
        ::NeoWPF::Diagnostics::LogError("RegisterFontMemory: CreateCustomFontFileReference failed 0x%08X", static_cast<unsigned int>(hr));
        return false;
    }

    BOOL supported = FALSE;
    DWRITE_FONT_FILE_TYPE fileType = DWRITE_FONT_FILE_TYPE_UNKNOWN;
    DWRITE_FONT_FACE_TYPE faceType = DWRITE_FONT_FACE_TYPE_UNKNOWN;
    UINT32 faceCount = 0;
    const HRESULT analyzeResult = fontFile->Analyze(&supported, &fileType, &faceType, &faceCount);
    if (FAILED(analyzeResult) || !supported || faceCount == 0) {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        g_memoryFonts.erase(
            std::remove_if(g_memoryFonts.begin(), g_memoryFonts.end(),
                [memoryFontKey](const MemoryFont& font) { return font.key == memoryFontKey; }),
            g_memoryFonts.end());
        ::NeoWPF::Diagnostics::LogError(
            "RegisterFontMemory: font analysis failed hr=0x%08X supported=%d faces=%u",
            static_cast<unsigned int>(analyzeResult),
            supported ? 1 : 0,
            faceCount);
        return false;
    }

    g_registeredFontFilePaths.push_back(L"memory://" + familyName);
    g_registeredFontFiles.push_back(fontFile);
    g_customFontCollection = nullptr;
    ::NeoWPF::Diagnostics::Log(
        ::NeoWPF::Diagnostics::Category::TextRendering,
        ::NeoWPF::Diagnostics::Level::Info,
        "RegisterFontMemory: '%ls' accepted by DirectWrite and added to custom collection",
        familyName.c_str());
    return true;
}

IDWriteFontFace* CreateRegisteredFontFace(const std::wstring& familyName)
{
    if (familyName.empty()) {
        return nullptr;
    }

    const auto factory = GetDWriteFactory();
    if (!factory) {
        return nullptr;
    }

    ComPtr<IDWriteFontFile> registeredFontFile;
    {
        std::lock_guard<std::mutex> lock(g_customFontMutex);
        for (std::size_t index = 0; index < g_registeredFontFilePaths.size(); ++index) {
            const auto& filePath = g_registeredFontFilePaths[index];
        const auto separator = filePath.find_last_of(L"\\/");
        std::wstring fileName = filePath.substr(separator == std::wstring::npos ? 0 : separator + 1);
        const auto extension = fileName.find_last_of(L'.');
        if (extension != std::wstring::npos) {
            fileName.resize(extension);
        }

        if (fileName.size() != familyName.size()) {
            continue;
        }

        bool matches = true;
        for (std::size_t character = 0; character < familyName.size(); ++character) {
            if (::towlower(fileName[character]) != ::towlower(familyName[character])) {
                matches = false;
                break;
            }
        }
        if (!matches || index >= g_registeredFontFiles.size()) {
            continue;
        }

            registeredFontFile = g_registeredFontFiles[index];
            break;
        }
    }

    if (!registeredFontFile) {
        return nullptr;
    }

    BOOL supported = FALSE;
    DWRITE_FONT_FILE_TYPE fileType = DWRITE_FONT_FILE_TYPE_UNKNOWN;
    DWRITE_FONT_FACE_TYPE faceType = DWRITE_FONT_FACE_TYPE_UNKNOWN;
    UINT32 faceCount = 0;
    if (FAILED(registeredFontFile->Analyze(
            &supported, &fileType, &faceType, &faceCount)) ||
        !supported || faceCount == 0) {
        return nullptr;
    }

    IDWriteFontFile* files[] = { registeredFontFile.Get() };
    IDWriteFontFace* fontFace = nullptr;
    const HRESULT result = factory->CreateFontFace(
        faceType, 1, files, 0, DWRITE_FONT_SIMULATIONS_NONE, &fontFace);
    return SUCCEEDED(result) ? fontFace : nullptr;
}

std::vector<std::wstring> GetRegisteredFontFiles()
{
    std::lock_guard<std::mutex> lock(g_customFontMutex);
    return g_registeredFontFilePaths;
}

}
