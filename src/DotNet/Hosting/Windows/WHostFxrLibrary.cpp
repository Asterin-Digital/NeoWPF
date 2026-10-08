#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrLibrary.h>

#include <Windows.h>
#include <hostfxr.h>

#include <string>

namespace NeoWPF::DotNet::Hosting::HostFxr {

namespace {

thread_local std::string errorMessage;
thread_local hostfxr_set_error_writer_fn setErrorWriterFunction;
thread_local hostfxr_error_writer_fn previousErrorWriter;

void HOSTFXR_CALLTYPE CaptureError(const char_t* message)
{
    if (!message)
        return;

    const auto length = static_cast<int>(std::char_traits<wchar_t>::length(message));
    const auto utf8Length = WideCharToMultiByte(CP_UTF8, 0, message, length, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0)
        return;

    errorMessage.resize(static_cast<std::size_t>(utf8Length));
    WideCharToMultiByte(CP_UTF8, 0, message, length, errorMessage.data(), utf8Length, nullptr, nullptr);
}

}

HostStatus HostFxrLibrary::Load(const std::filesystem::path& path)
{
    if (m_handle)
        return HostStatus(HostStatusCode::Ok);

    const auto library = LoadLibraryW(path.c_str());
    if (!library)
    {
        return HostStatus(HostStatusCode::HostFxrLoadFailed)
            .SetMessage("The hostfxr library could not be loaded.")
            .SetHint(path.string());
    }

    m_handle = library;
    return HostStatus(HostStatusCode::Ok);
}

bool HostFxrLibrary::HasExport(const char* name) const noexcept
{
    return GetExport(name) != nullptr;
}

void* HostFxrLibrary::GetExport(const char* name) const noexcept
{
    return m_handle ? reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_handle), name)) : nullptr;
}

void HostFxrLibrary::SetErrorWriter(void* setErrorWriter) noexcept
{
    setErrorWriterFunction = reinterpret_cast<hostfxr_set_error_writer_fn>(setErrorWriter);
    errorMessage.clear();
    if (setErrorWriterFunction)
        previousErrorWriter = setErrorWriterFunction(&CaptureError);
}

void HostFxrLibrary::ClearErrorWriter() noexcept
{
    if (setErrorWriterFunction)
        setErrorWriterFunction(previousErrorWriter);
    setErrorWriterFunction = nullptr;
    previousErrorWriter = nullptr;
}

const std::string& HostFxrLibrary::GetErrorMessage() const noexcept
{
    return errorMessage;
}

}