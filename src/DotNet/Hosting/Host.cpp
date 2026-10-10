#include <NeoWPF/DotNet/Hosting/Host.h>

#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <coreclr_delegates.h>
#include <hostfxr.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <functional>
#include <string>

namespace NeoWPF::DotNet::Hosting {

namespace {

std::atomic<std::uint32_t> nextContextId{ 1 };

void NW_CALL CaptureManagedLog(
    void* userData,
    std::uint32_t level,
    NwStringView category,
    NwStringView message)
{
    auto* log = static_cast<std::function<void(std::uint32_t, const std::string&, const std::string&)>*>(userData);
    if (!log || !*log)
        return;

    try
    {
        const std::string categoryText(
            reinterpret_cast<const char*>(category.data),
            static_cast<std::size_t>(category.length));
        const std::string messageText(
            reinterpret_cast<const char*>(message.data),
            static_cast<std::size_t>(message.length));
        (*log)(level, categoryText, messageText);
    }
    catch (...)
    {
    }
}

bool IsSuccessfulInitialization(std::int32_t result) noexcept
{
    return result == 0 || result == 1 || result == 2;
}

struct FrameworkResolutionState {
    std::size_t unresolvedCount = 0;
};

void HOSTFXR_CALLTYPE CaptureFrameworkResolution(
    const hostfxr_resolve_frameworks_result* result,
    void* context)
{
    if (result && context)
        static_cast<FrameworkResolutionState*>(context)->unresolvedCount = result->unresolved_count;
}

bool HasUnresolvedFrameworks(
    HostFxr::HostFxrLibrary& library,
    const std::filesystem::path& runtimeConfigPath,
    const std::filesystem::path& hostFxrPath,
    const std::filesystem::path& dotnetRoot)
{
    const auto resolveFrameworks = reinterpret_cast<hostfxr_resolve_frameworks_for_runtime_config_fn>(
        library.GetExport("hostfxr_resolve_frameworks_for_runtime_config"));
    if (!resolveFrameworks)
        return false;

    hostfxr_initialize_parameters parameters{};
    parameters.size = sizeof(parameters);
    parameters.host_path = hostFxrPath.c_str();
    parameters.dotnet_root = dotnetRoot.empty() ? nullptr : dotnetRoot.c_str();
    FrameworkResolutionState state;

    const auto result = resolveFrameworks(
        runtimeConfigPath.c_str(),
        &parameters,
        &CaptureFrameworkResolution,
        &state);
    return result == 0 && state.unresolvedCount != 0;
}

HostStatus RuntimeInitializationFailure(
    HostFxr::HostFxrLibrary& library,
    std::int32_t result,
    bool frameworkMissing)
{
    const auto message = library.GetErrorMessage();
    library.ClearErrorWriter();
    return HostStatus(frameworkMissing ? HostStatusCode::FrameworkMissing : HostStatusCode::RuntimeInitFailed)
        .SetRawHostFxrResult(result)
        .SetMessage(message.empty() ? "hostfxr could not initialize the .NET runtime." : message)
        .SetHint("Install the .NET 10 runtime or later.");
}

bool IsFrameworkMissingDiagnostic(const std::string& message)
{
    std::string lowercaseMessage = message;
    std::ranges::transform(lowercaseMessage, lowercaseMessage.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });

    return lowercaseMessage.find("missing framework") != std::string::npos ||
        lowercaseMessage.find("framework is missing") != std::string::npos ||
        lowercaseMessage.find("framework was not found") != std::string::npos;
}

}

HostStatus Host::Start(const HostOptions& options)
{
    if (m_session.IsAlive())
        return HostStatus(HostStatusCode::AlreadyStarted);
    m_log = options.log;

    std::filesystem::path bridgeDirectory;
    if (!m_runtimeDelegate)
    {
        const auto hostStatus = ResolveHostFxrPathStatus(options);
        if (!hostStatus.Succeeded())
            return hostStatus;

        const auto hostFxrPath = ResolveHostFxrPath(options);
        const auto loadStatus = m_hostFxrLibrary.Load(hostFxrPath);
        if (!loadStatus.Succeeded())
            return loadStatus;

        constexpr std::array<const char*, 3> requiredExports{
            "hostfxr_initialize_for_runtime_config",
            "hostfxr_get_runtime_delegate",
            "hostfxr_close"
        };
        for (const auto* exportName : requiredExports)
        {
            if (!m_hostFxrLibrary.HasExport(exportName))
            {
                return HostStatus(HostStatusCode::HostFxrSymbolMissing)
                    .SetMessage("A required hostfxr export is missing.")
                    .SetHint(exportName);
            }
        }

        bridgeDirectory = HostFxr::ResolveLocation(
            HostFxr::GetExecutableDirectory(),
            options.bridgeDirectory);
        if (bridgeDirectory.empty())
        {
            return HostStatus(HostStatusCode::BridgeMissing)
                .SetMessage("The managed bridge files could not be located.")
                .SetHint("Deploy NeoWPF.DotNet.Bridge next to the application or set bridgeDirectory.");
        }

        const auto runtimeConfigPath = bridgeDirectory / "NeoWPF.DotNet.Bridge.runtimeconfig.json";
        m_hostFxrLibrary.SetErrorWriter(m_hostFxrLibrary.GetExport("hostfxr_set_error_writer"));
        const auto initialize = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
            m_hostFxrLibrary.GetExport("hostfxr_initialize_for_runtime_config"));
        const auto getRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(
            m_hostFxrLibrary.GetExport("hostfxr_get_runtime_delegate"));
        const auto close = reinterpret_cast<hostfxr_close_fn>(
            m_hostFxrLibrary.GetExport("hostfxr_close"));

        hostfxr_initialize_parameters initializationParameters{};
        initializationParameters.size = sizeof(initializationParameters);
        initializationParameters.host_path = hostFxrPath.c_str();
        initializationParameters.dotnet_root = options.dotnetRoot.empty() ? nullptr : options.dotnetRoot.c_str();

        hostfxr_handle context = nullptr;
        const auto initializeResult = initialize(
            runtimeConfigPath.c_str(),
            &initializationParameters,
            &context);
        if (!IsSuccessfulInitialization(initializeResult) || !context)
        {
            if (context)
                close(context);
            const auto frameworkMissing = HasUnresolvedFrameworks(
                m_hostFxrLibrary,
                runtimeConfigPath,
                hostFxrPath,
                options.dotnetRoot) || IsFrameworkMissingDiagnostic(m_hostFxrLibrary.GetErrorMessage());
            return RuntimeInitializationFailure(m_hostFxrLibrary, initializeResult, frameworkMissing);
        }

        void* runtimeDelegate = nullptr;
        const auto delegateResult = getRuntimeDelegate(
            context,
            hdt_load_assembly_and_get_function_pointer,
            &runtimeDelegate);
        const auto closeResult = close(context);
        if (delegateResult != 0 || closeResult != 0 || !runtimeDelegate)
        {
            const auto result = delegateResult != 0 ? delegateResult : closeResult;
            const auto message = m_hostFxrLibrary.GetErrorMessage();
            m_hostFxrLibrary.ClearErrorWriter();
            return HostStatus(HostStatusCode::RuntimeInitFailed)
                .SetRawHostFxrResult(result)
                .SetMessage(message.empty() ? "hostfxr could not acquire the assembly-loading delegate." : message)
                .SetHint("Install the .NET 10 runtime or later.");
        }

        m_hostFxrLibrary.ClearErrorWriter();
        m_runtimeDelegate = runtimeDelegate;
    }

    if (bridgeDirectory.empty())
    {
        bridgeDirectory = HostFxr::ResolveLocation(
            HostFxr::GetExecutableDirectory(),
            options.bridgeDirectory);
        if (bridgeDirectory.empty())
        {
            return HostStatus(HostStatusCode::BridgeMissing)
                .SetMessage("The managed bridge files could not be located.")
                .SetHint("Deploy NeoWPF.DotNet.Bridge next to the application or set bridgeDirectory.");
        }
    }

    const auto loadAssembly = reinterpret_cast<load_assembly_and_get_function_pointer_fn>(m_runtimeDelegate);
    const auto bridgeAssembly = bridgeDirectory / "NeoWPF.DotNet.Bridge.dll";
    const std::filesystem::path bridgeType(
        "NeoWPF.DotNet.Bridge.Runtime.EntryPoint, NeoWPF.DotNet.Bridge");
    const std::filesystem::path bridgeMethod("NwBridgeInitialize");
    void* bridgeEntryPoint = nullptr;
    const auto bridgeLoadResult = loadAssembly(
        bridgeAssembly.c_str(),
        bridgeType.c_str(),
        bridgeMethod.c_str(),
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        &bridgeEntryPoint);
    if (bridgeLoadResult != 0 || !bridgeEntryPoint)
    {
        return HostStatus(HostStatusCode::BridgeLoadFailed)
            .SetMessage("The managed bridge entry point could not be loaded.")
            .SetHint(bridgeAssembly.string());
    }

    auto bridgeInitialize = reinterpret_cast<NwBridgeInitializeFn>(bridgeEntryPoint);
    NwHostCallbacks callbacks{};
    callbacks.structSize = sizeof(callbacks);
    callbacks.abiVersion = NW_ABI_VERSION;
    if (m_log)
    {
        callbacks.userData = &m_log;
        callbacks.log = &CaptureManagedLog;
    }
    const auto contextId = nextContextId.fetch_add(1, std::memory_order_relaxed);
    if (contextId == 0)
    {
        return HostStatus(HostStatusCode::RuntimeInitFailed)
            .SetMessage("The host session identifier space is exhausted.");
    }

    NwBridgeInitInfo initInfo{};
    initInfo.structSize = sizeof(initInfo);
    initInfo.abiVersion = NW_ABI_VERSION;
    initInfo.contextId = contextId;
    initInfo.host = &callbacks;

    const NwBridgeApi* bridgeApi = nullptr;
    const auto bridgeStatus = bridgeInitialize(&initInfo, &bridgeApi);
    if (bridgeStatus != NwStatus_Ok || !bridgeApi)
    {
        return HostStatus(bridgeStatus == NwStatus_AbiMismatch
                ? HostStatusCode::AbiMismatch
                : HostStatusCode::BridgeLoadFailed)
            .SetMessage("The managed bridge rejected initialization.");
    }

    constexpr auto requiredApiSize = offsetof(NwBridgeApi, Shutdown) + sizeof(NwBridgeApi::Shutdown);
    if ((bridgeApi->abiVersion >> 16) != NW_ABI_MAJOR ||
        bridgeApi->structSize < requiredApiSize ||
        !bridgeApi->Shutdown)
    {
        return HostStatus(HostStatusCode::AbiMismatch)
            .SetMessage("The managed bridge function table is incompatible.");
    }

    m_session.Start(contextId, *bridgeApi);
    return HostStatus(HostStatusCode::Ok);
}

bool Host::Stop()
{
    if (!m_session.IsAlive())
        return false;

    m_session.Stop(0);
    m_log = {};
    return true;
}

const HostSession& Host::GetSession() const noexcept
{
    return m_session;
}

std::filesystem::path Host::ResolveHostFxrPath(const HostOptions& options)
{
    return HostFxr::ResolvePath(options);
}

HostStatus Host::ResolveHostFxrPathStatus(const HostOptions& options)
{
    return HostFxr::ResolveStatus(options);
}

std::filesystem::path Host::ResolveManagedBridgeDirectory(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& bridgeDirectory)
{
    return HostFxr::ResolveLocation(executableDirectory, bridgeDirectory);
}

}
