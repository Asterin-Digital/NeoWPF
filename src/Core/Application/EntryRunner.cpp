#include <NeoWPF/Core/Application/EntryRunner.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/Entry.h>
#include <NeoWPF/Core/Application/LaunchContext.h>
#include <NeoWPF/Diagnostics/Logger.h>

#include <exception>
#include <utility>

namespace NeoWPF::Core {

LaunchContext::LaunchContext(
    NeoWPF::Result<std::vector<std::wstring>> arguments,
    NeoWPF::Result<std::optional<std::string>> dataUri,
    NeoWPF::Result<std::filesystem::path> executableDirectory)
    : m_arguments(std::move(arguments)),
      m_dataUri(std::move(dataUri)),
      m_executableDirectory(std::move(executableDirectory))
{
}

const NeoWPF::Result<std::vector<std::wstring>>& LaunchContext::Arguments() const noexcept
{
    return m_arguments;
}

const NeoWPF::Result<std::optional<std::string>>& LaunchContext::DataUri() const noexcept
{
    return m_dataUri;
}

const NeoWPF::Result<std::filesystem::path>& LaunchContext::ExecutableDirectory() const noexcept
{
    return m_executableDirectory;
}

int EntryRunner::Run(
    NeoWPF::Result<std::vector<std::wstring>> arguments,
    NeoWPF::Result<std::optional<std::string>> dataUri,
    NeoWPF::Result<std::filesystem::path> executableDirectory)
{
    int exitCode = 1;
    try {
        const LaunchContext launch(
            std::move(arguments),
            std::move(dataUri),
            std::move(executableDirectory));
        try {
            exitCode = Entry(launch);
        } catch (const std::exception& exception) {
            NeoWPF::Diagnostics::Catastrophic(
                "Application entry threw an exception: %s", exception.what());
        } catch (...) {
            NeoWPF::Diagnostics::Catastrophic(
                "Application entry threw an unknown exception");
        }
    } catch (const std::exception& exception) {
        NeoWPF::Diagnostics::Catastrophic(
            "Failed to create application launch context: %s", exception.what());
    } catch (...) {
        NeoWPF::Diagnostics::Catastrophic(
            "Failed to create application launch context");
    }

    try {
        Application::Shutdown();
    } catch (const std::exception& exception) {
        NeoWPF::Diagnostics::Catastrophic(
            "Application shutdown failed: %s", exception.what());
        exitCode = exitCode == 0 ? 1 : exitCode;
    } catch (...) {
        NeoWPF::Diagnostics::Catastrophic(
            "Application shutdown failed with an unknown exception");
        exitCode = exitCode == 0 ? 1 : exitCode;
    }
    return exitCode;
}

}
