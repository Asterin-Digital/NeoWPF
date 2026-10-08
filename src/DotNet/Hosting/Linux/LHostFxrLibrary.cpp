#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrLibrary.h>

#include <dlfcn.h>
#include <hostfxr.h>

#include <string>

namespace NeoWPF::DotNet::Hosting::HostFxr {

namespace {

thread_local std::string errorMessage;
thread_local hostfxr_set_error_writer_fn setErrorWriterFunction;
thread_local hostfxr_error_writer_fn previousErrorWriter;

void HOSTFXR_CALLTYPE CaptureError(const char_t* message)
{
    if (message)
        errorMessage = message;
}

}

HostStatus HostFxrLibrary::Load(const std::filesystem::path& path)
{
    if (m_handle)
        return HostStatus(HostStatusCode::Ok);

    m_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!m_handle)
    {
        const auto* error = dlerror();
        return HostStatus(HostStatusCode::HostFxrLoadFailed)
            .SetMessage("The hostfxr library could not be loaded.")
            .SetHint(error ? error : path.string());
    }

    return HostStatus(HostStatusCode::Ok);
}

bool HostFxrLibrary::HasExport(const char* name) const noexcept
{
    return GetExport(name) != nullptr;
}

void* HostFxrLibrary::GetExport(const char* name) const noexcept
{
    if (!m_handle)
        return nullptr;

    dlerror();
    return dlsym(m_handle, name);
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