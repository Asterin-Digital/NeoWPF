#include <NeoWPF/DotNet/Hosting/HostSession.h>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

namespace NeoWPF::DotNet::Hosting {

namespace {

constexpr std::uint32_t ReferenceValueKind = NwValueKind_Reference;

NwReference ToAbiReference(const auto& reference)
{
    return {
        reference.GetContextId(),
        reference.GetIndex(),
        reference.GetGeneration(),
        static_cast<std::uint32_t>(reference.GetKind())
    };
}

NwStringView ToAbiString(std::string_view value)
{
    return {
        reinterpret_cast<const std::uint8_t*>(value.data()),
        value.size()
    };
}

void InitializeResult(NwResult& result) noexcept
{
    result = {};
    result.structSize = sizeof(result);
    result.abiVersion = NW_ABI_VERSION;
}

std::string GetFailureMessage(const NwResult& result, NwStatus status)
{
    const auto& message = result.error.message;
    if (message.data && message.length <= std::numeric_limits<std::size_t>::max())
    {
        return {
            reinterpret_cast<const char*>(message.data),
            static_cast<std::size_t>(message.length)
        };
    }

    return "Managed bridge call failed with status " + std::to_string(status) + ".";
}

class ResultRelease final {
public:
    ResultRelease(const NwBridgeApi* api, NwResult& result) noexcept
        : m_api(api), m_result(&result)
    {
    }

    ~ResultRelease()
    {
        if (m_api && m_api->ReleaseResult)
            m_api->ReleaseResult(m_result);
    }

private:
    const NwBridgeApi* m_api;
    NwResult* m_result;
};

ManagedResult<NwReference> GetReferenceResult(
    const NwBridgeApi* api,
    NwResult& result,
    NwStatus callStatus,
    ManagedReferenceKind expectedKind)
{
    ResultRelease release(api, result);
    const auto status = callStatus == NwStatus_Ok ? result.status : callStatus;
    if (status != NwStatus_Ok)
        return ManagedResult<NwReference>::Error(GetFailureMessage(result, status));

    const auto reference = result.value.payload.reference;
    if (result.value.kind != ReferenceValueKind ||
        reference.kind != static_cast<std::uint32_t>(expectedKind))
    {
        return ManagedResult<NwReference>::Error("The managed bridge returned an invalid reference.");
    }

    return ManagedResult<NwReference>::Success(reference);
}

ManagedResult<void> GetVoidResult(
    const NwBridgeApi* api,
    NwResult& result,
    NwStatus callStatus)
{
    ResultRelease release(api, result);
    const auto status = callStatus == NwStatus_Ok ? result.status : callStatus;
    if (status != NwStatus_Ok)
        return ManagedResult<void>::Error(GetFailureMessage(result, status));
    return ManagedResult<void>::Success();
}

}

bool HostSession::IsAlive() const noexcept
{
    return m_contextId != 0;
}

std::uint32_t HostSession::GetContextId() const noexcept
{
    return m_contextId;
}

const NwBridgeApi* HostSession::GetBridgeApi() const noexcept
{
    return IsAlive() ? &m_bridgeApi : nullptr;
}

ManagedResult<ManagedAssemblyReference> HostSession::LoadAssembly(std::string_view path) const
{
    const auto* bridgeApi = GetBridgeApi();
    if (!bridgeApi || !bridgeApi->LoadAssembly || !bridgeApi->ReleaseResult)
        return ManagedResult<ManagedAssemblyReference>::Error("The managed host session is stopped.");

    NwLoadAssemblyRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.path = ToAbiString(path);
    NwResult result{};
    InitializeResult(result);
    const auto status = bridgeApi->LoadAssembly(&request, &result);
    auto reference = GetReferenceResult(bridgeApi, result, status, ManagedReferenceKind::Assembly);
    if (!reference.Succeeded())
        return ManagedResult<ManagedAssemblyReference>::Error(reference.GetMessage());

    const auto& abiReference = reference.GetValue().value();
    return ManagedResult<ManagedAssemblyReference>::Success({
        abiReference.contextId,
        abiReference.index,
        abiReference.generation
    });
}

ManagedResult<void> HostSession::VerifyContract(
    std::string_view bindingName,
    std::span<const std::uint8_t, 32> fingerprint,
    std::span<const std::uint8_t> assemblyMvids) const
{
    const auto* bridgeApi = GetBridgeApi();
    if (!bridgeApi || !bridgeApi->VerifyContract || !bridgeApi->ReleaseResult)
        return ManagedResult<void>::Error("The managed host session is stopped.");
    if (assemblyMvids.empty() || assemblyMvids.size() % 16 != 0 ||
        assemblyMvids.size() / 16 > std::numeric_limits<std::uint32_t>::max())
    {
        return ManagedResult<void>::Error("The assembly MVID list is invalid.");
    }

    NwVerifyContractRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.bindingName = ToAbiString(bindingName);
    request.fingerprint = fingerprint.data();
    request.assemblyCount = static_cast<std::uint32_t>(assemblyMvids.size() / 16);
    request.assemblyMvids = assemblyMvids.data();
    NwResult result{};
    InitializeResult(result);
    return GetVoidResult(bridgeApi, result, bridgeApi->VerifyContract(&request, &result));
}

ManagedResult<ManagedTypeReference> HostSession::ResolveType(
    std::string_view fullName,
    ManagedAssemblyReference assembly) const
{
    const auto* bridgeApi = GetBridgeApi();
    if (!bridgeApi || !bridgeApi->ResolveType || !bridgeApi->ReleaseResult)
        return ManagedResult<ManagedTypeReference>::Error("The managed host session is stopped.");

    NwResolveTypeRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    if (!assembly.IsNull())
        request.assembly = ToAbiReference(assembly);
    request.fullName = ToAbiString(fullName);
    NwResult result{};
    InitializeResult(result);
    const auto status = bridgeApi->ResolveType(&request, &result);
    auto reference = GetReferenceResult(bridgeApi, result, status, ManagedReferenceKind::Type);
    if (!reference.Succeeded())
        return ManagedResult<ManagedTypeReference>::Error(reference.GetMessage());

    const auto& abiReference = reference.GetValue().value();
    return ManagedResult<ManagedTypeReference>::Success({
        abiReference.contextId,
        abiReference.index,
        abiReference.generation
    });
}

ManagedResult<ManagedMemberReference> HostSession::ResolveMember(
    const ManagedTypeReference& type,
    std::string_view name,
    std::span<const ManagedTypeReference> parameterTypes,
    bool isStatic) const
{
    const auto* bridgeApi = GetBridgeApi();
    if (!bridgeApi || !bridgeApi->ResolveMember || !bridgeApi->ReleaseResult)
        return ManagedResult<ManagedMemberReference>::Error("The managed host session is stopped.");
    if (parameterTypes.size() > std::numeric_limits<std::uint32_t>::max())
        return ManagedResult<ManagedMemberReference>::Error("The method parameter count is too large.");

    std::vector<NwParameterDescriptor> parameters;
    parameters.reserve(parameterTypes.size());
    for (const auto& parameterType : parameterTypes)
        parameters.push_back({ ToAbiReference(parameterType), 0, 0 });

    NwResolveMemberRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.type = ToAbiReference(type);
    request.name = ToAbiString(name);
    request.memberKind = 1;
    request.flags = isStatic ? 1u : 0u;
    request.parameterCount = static_cast<std::uint32_t>(parameters.size());
    request.parameters = parameters.data();
    NwResult result{};
    InitializeResult(result);
    const auto status = bridgeApi->ResolveMember(&request, &result);
    auto reference = GetReferenceResult(bridgeApi, result, status, ManagedReferenceKind::Member);
    if (!reference.Succeeded())
        return ManagedResult<ManagedMemberReference>::Error(reference.GetMessage());

    const auto& abiReference = reference.GetValue().value();
    return ManagedResult<ManagedMemberReference>::Success({
        abiReference.contextId,
        abiReference.index,
        abiReference.generation
    });
}

ManagedResult<std::int32_t> HostSession::InvokeInt32(
    const ManagedMemberReference& member,
    std::int32_t argument) const
{
    const auto* bridgeApi = GetBridgeApi();
    if (!bridgeApi || !bridgeApi->InvokeMember || !bridgeApi->ReleaseResult)
        return ManagedResult<std::int32_t>::Error("The managed host session is stopped.");

    NwValue input{};
    input.kind = NwValueKind_Int32;
    input.payload.i64 = argument;
    NwInvokeRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.member = ToAbiReference(member);
    request.arguments = &input;
    request.argumentCount = 1;
    request.policy = 0;
    NwResult result{};
    InitializeResult(result);
    const auto callStatus = bridgeApi->InvokeMember(&request, &result);
    ResultRelease release(bridgeApi, result);
    const auto status = callStatus == NwStatus_Ok ? result.status : callStatus;
    if (status != NwStatus_Ok)
        return ManagedResult<std::int32_t>::Error(GetFailureMessage(result, status));
    if (result.value.kind != NwValueKind_Int32)
        return ManagedResult<std::int32_t>::Error("The managed method returned a non-int32 value.");

    return ManagedResult<std::int32_t>::Success(static_cast<std::int32_t>(result.value.payload.i64));
}

void HostSession::Start(std::uint32_t contextId, const NwBridgeApi& bridgeApi) noexcept
{
    m_contextId = contextId;
    m_bridgeApi = bridgeApi;
}

void HostSession::Stop(std::uint32_t drainTimeoutMilliseconds) noexcept
{
    if (!IsAlive())
        return;

    m_bridgeApi.Shutdown(drainTimeoutMilliseconds);
    m_contextId = 0;
    m_bridgeApi = {};
}

}