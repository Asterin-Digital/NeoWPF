#include <gtest/gtest.h>

#include <NeoWPF/DotNet/Hosting/Host.h>
#include <NeoWPF/DotNet/Hosting/HostOptions.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrLibrary.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/HostFxrPath.h>
#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>
#include "BasicAssembly.g.h"

#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace {

void CreateBridgeFiles(const std::filesystem::path& directory)
{
    std::filesystem::create_directories(directory);
    constexpr std::array<const char*, 3> bridgeFiles{
        "NeoWPF.DotNet.Bridge.dll",
        "NeoWPF.DotNet.Bridge.runtimeconfig.json",
        "NeoWPF.DotNet.Bridge.deps.json"
    };

    for (const auto* file : bridgeFiles)
        std::ofstream(directory / file).put(' ');
}

struct OperationCompletion final
{
    std::mutex mutex;
    std::condition_variable condition;
    NwReference reference{};
    std::uint32_t state = 0;
    bool completed = false;

    bool Wait()
    {
        std::unique_lock lock(mutex);
        return condition.wait_for(lock, std::chrono::seconds(5), [this] { return completed; });
    }
};

void NW_CALL CompleteOperation(void* userData, NwReference operation, std::uint32_t state)
{
    auto& completion = *static_cast<OperationCompletion*>(userData);
    {
        std::lock_guard lock(completion.mutex);
        completion.reference = operation;
        completion.state = state;
        completion.completed = true;
    }
    completion.condition.notify_one();
}

NwReference StartAsyncCall(
    const NeoWPF::DotNet::Hosting::HostSession& session,
    const NeoWPF::DotNet::ManagedTypeReference& type,
    const char* methodName,
    std::int32_t argumentValue,
    OperationCompletion& completion,
    NwStatus& callStatus)
{
    const auto* api = session.GetBridgeApi();
    if (!api || !api->InvokeMember)
    {
        callStatus = NwStatus_SessionEnded;
        return {};
    }

    NwResolveMemberRequest memberRequest{};
    memberRequest.header = { sizeof(memberRequest), NW_ABI_VERSION };
    memberRequest.type = {
        type.GetContextId(),
        type.GetIndex(),
        type.GetGeneration(),
        static_cast<std::uint32_t>(type.GetKind())
    };
    memberRequest.name = {
        reinterpret_cast<const std::uint8_t*>(methodName),
        std::strlen(methodName)
    };
    memberRequest.memberKind = 1;
    memberRequest.flags = 1 | 2;
    NwResult memberResult{};
    callStatus = api->ResolveMember(&memberRequest, &memberResult);
    if (callStatus != NwStatus_Ok)
    {
        api->ReleaseResult(&memberResult);
        return {};
    }
    const auto memberReference = memberResult.value.payload.reference;
    api->ReleaseResult(&memberResult);

    NwValue argument{};
    argument.kind = NwValueKind_Int32;
    argument.payload.i64 = argumentValue;
    NwOperationSink sink{ &completion, &CompleteOperation };
    NwInvokeRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.member = memberReference;
    request.arguments = &argument;
    request.argumentCount = 1;
    request.policy = NwExecutionPolicy_Managed;
    request.sink = &sink;
    NwResult result{};
    callStatus = api->InvokeMember(&request, &result);
    const auto operation = result.value.payload.reference;
    api->ReleaseResult(&result);
    return operation;
}

TEST(DotNetHostTests, ExplicitHostFxrPathHasPriority)
{
    const std::filesystem::path explicitPath = "C:/runtime/hostfxr.dll";
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = explicitPath,
        .dotnetRoot = "C:/dotnet"
    };

    EXPECT_EQ(NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(options), explicitPath);
}

TEST(DotNetHostTests, HostStatusPreservesRawHostFxrResult)
{
    NeoWPF::DotNet::Hosting::HostStatus status(
        NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeInitFailed);

    EXPECT_EQ(status.GetRawHostFxrResult(), 0);
    status.SetRawHostFxrResult(-2147450749);
    EXPECT_EQ(status.GetRawHostFxrResult(), -2147450749);
}

TEST(DotNetHostTests, AbiEnumValuesMatchGoldenNumbers)
{
    EXPECT_EQ(NwStatus_Ok, 0);
    EXPECT_EQ(NwStatus_InvalidArgument, 1);
    EXPECT_EQ(NwStatus_AbiMismatch, 2);
    EXPECT_EQ(NwStatus_WrongContext, 3);
    EXPECT_EQ(NwStatus_StaleReference, 4);
    EXPECT_EQ(NwStatus_WrongKind, 5);
    EXPECT_EQ(NwStatus_NotFound, 6);
    EXPECT_EQ(NwStatus_Ambiguous, 7);
    EXPECT_EQ(NwStatus_SignatureMismatch, 8);
    EXPECT_EQ(NwStatus_ConversionFailed, 9);
    EXPECT_EQ(NwStatus_ManagedException, 10);
    EXPECT_EQ(NwStatus_WrongThread, 11);
    EXPECT_EQ(NwStatus_RenderThreadForbidden, 12);
    EXPECT_EQ(NwStatus_Canceled, 13);
    EXPECT_EQ(NwStatus_Unsupported, 14);
    EXPECT_EQ(NwStatus_ContractMismatch, 15);
    EXPECT_EQ(NwStatus_SessionEnded, 16);
    EXPECT_EQ(NwStatus_OutOfMemory, 17);
    EXPECT_EQ(NwStatus_InternalError, 18);
    EXPECT_EQ(NwReferenceKind_None, 0);
    EXPECT_EQ(NwReferenceKind_Assembly, 1);
    EXPECT_EQ(NwReferenceKind_Type, 2);
    EXPECT_EQ(NwReferenceKind_Member, 3);
    EXPECT_EQ(NwReferenceKind_Object, 4);
    EXPECT_EQ(NwReferenceKind_Delegate, 5);
    EXPECT_EQ(NwReferenceKind_Exception, 6);
    EXPECT_EQ(NwReferenceKind_Operation, 7);
    EXPECT_EQ(NwValueKind_Null, 0);
    EXPECT_EQ(NwValueKind_Bool, 1);
    EXPECT_EQ(NwValueKind_Int32, 4);
    EXPECT_EQ(NwValueKind_Reference, 16);
    EXPECT_EQ(NwValueKind_Array, 17);
    EXPECT_EQ(NwExecutionPolicy_Unspecified, 0);
    EXPECT_EQ(NwExecutionPolicy_Managed, 1);
    EXPECT_EQ(NwExecutionPolicy_Calling, 2);
    EXPECT_EQ(NwExecutionPolicy_Application, 3);
    EXPECT_EQ(NwExecutionPolicy_Background, 4);
}

TEST(DotNetHostTests, MissingHostFxrReturnsRuntimeNotFound)
{
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = std::filesystem::path(),
        .dotnetRoot = std::filesystem::path("C:/missing-dotnet")
    };

    const auto status = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPathStatus(options);
    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound);
}

TEST(DotNetHostTests, HostStartReportsRuntimeMissingBeforeBridgeMissing)
{
    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = std::filesystem::path("C:/missing-dotnet/hostfxr.dll"),
        .bridgeDirectory = std::filesystem::path("C:/missing-bridge")
    };

    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound);
}

TEST(DotNetHostTests, HostFxrLibraryRejectsInvalidLibraryFile)
{
    const auto hostfxrPath = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.invalid-hostfxr";
    std::ofstream(hostfxrPath).put(' ');

    NeoWPF::DotNet::Hosting::HostFxr::HostFxrLibrary library;
    const auto status = library.Load(hostfxrPath);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrLoadFailed);
    std::filesystem::remove(hostfxrPath);
}

TEST(DotNetHostTests, NethostFindsAndLoadsInstalledHostFxr)
{
    const NeoWPF::DotNet::Hosting::HostOptions options;
    const auto path = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(options);
    if (path.empty())
        GTEST_SKIP() << "No .NET runtime is installed.";

    NeoWPF::DotNet::Hosting::HostFxr::HostFxrLibrary library;
    const auto status = library.Load(path);
    if (!status.Succeeded())
        GTEST_SKIP() << status.Message();

    EXPECT_TRUE(library.HasExport("hostfxr_initialize_for_runtime_config"));
    EXPECT_TRUE(library.HasExport("hostfxr_get_runtime_delegate"));
    EXPECT_TRUE(library.HasExport("hostfxr_close"));
    EXPECT_FALSE(library.HasExport("NeoWPF_Missing_HostFxr_Export"));
}

TEST(DotNetHostTests, FindsCurrentExecutableDirectory)
{
    const auto executableDirectory = NeoWPF::DotNet::Hosting::HostFxr::GetExecutableDirectory();

    ASSERT_FALSE(executableDirectory.empty());
    EXPECT_TRUE(std::filesystem::is_directory(executableDirectory));
}

TEST(DotNetHostTests, HostStartInitializesManagedBridge)
{
    const NeoWPF::DotNet::Hosting::HostOptions defaultOptions;
    const auto hostfxrPath = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath(defaultOptions);
    if (hostfxrPath.empty())
        GTEST_SKIP() << "No .NET runtime is installed.";

    const auto bridgeDirectory = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.empty-bridge";
    std::filesystem::create_directories(bridgeDirectory);

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath,
        .bridgeDirectory = bridgeDirectory
    };
    const auto status = host.Start(options);

    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }

    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();
    EXPECT_TRUE(host.GetSession().IsAlive());
    const auto firstContextId = host.GetSession().GetContextId();
    EXPECT_NE(firstContextId, 0u);
    EXPECT_TRUE(host.Stop());
    EXPECT_FALSE(host.GetSession().IsAlive());
    EXPECT_EQ(host.GetSession().GetBridgeApi(), nullptr);

    const auto restartStatus = host.Start(options);
    ASSERT_EQ(restartStatus.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok)
        << restartStatus.Message();
    EXPECT_NE(host.GetSession().GetContextId(), firstContextId);
    EXPECT_TRUE(host.Stop());
    std::filesystem::remove_all(bridgeDirectory);
}

TEST(DotNetHostTests, GeneratedBindingInvokesStaticInt32Method)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }

    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();
    const auto binding = BasicAssembly::InvoiceServiceBinding::Load(
        host.GetSession(),
        NEOWPF_DOTNET_BINDING_DIRECTORY);
    if (!binding.Succeeded())
    {
        host.Stop();
        FAIL() << binding.GetMessage();
    }

    const auto result = binding.GetValue()->AddTen(5);
    EXPECT_TRUE(result.Succeeded()) << result.GetMessage();
    if (result.Succeeded())
        EXPECT_EQ(result.GetValue().value(), 15);

    const auto utilityBinding = BasicAssembly::UtilityServiceBinding::Load(
        host.GetSession(),
        NEOWPF_DOTNET_BINDING_DIRECTORY);
    ASSERT_TRUE(utilityBinding.Succeeded()) << utilityBinding.GetMessage();
    const auto doubled = utilityBinding.GetValue()->Double(5);
    EXPECT_TRUE(doubled.Succeeded()) << doubled.GetMessage();
    if (doubled.Succeeded())
        EXPECT_EQ(doubled.GetValue().value(), 10);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, IsolatedContextResolvesDependenciesPerAssemblyPath)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    const auto fixturePath = (
        std::filesystem::path(NEOWPF_DOTNET_BINDING_DIRECTORY) / "managed" / "BasicAssembly.dll").string();
    auto fixtureAssembly = host.GetSession().LoadAssembly(fixturePath);
    ASSERT_TRUE(fixtureAssembly.Succeeded()) << fixtureAssembly.GetMessage();

    const auto pluginPath = (
        std::filesystem::path(NEOWPF_DOTNET_PLUGIN_DIRECTORY) / "PluginAssembly.dll").string();
    auto pluginAssembly = host.GetSession().LoadAssembly(pluginPath);
    ASSERT_TRUE(pluginAssembly.Succeeded()) << pluginAssembly.GetMessage();

    auto type = host.GetSession().ResolveType("PluginAssembly.PluginService", *pluginAssembly.GetValue());
    ASSERT_TRUE(type.Succeeded()) << type.GetMessage();
    auto int32Type = host.GetSession().ResolveType("System.Int32");
    ASSERT_TRUE(int32Type.Succeeded()) << int32Type.GetMessage();
    const std::array parameterTypes{ *int32Type.GetValue() };
    auto member = host.GetSession().ResolveMember(*type.GetValue(), "AddFive", parameterTypes, true);
    ASSERT_TRUE(member.Succeeded()) << member.GetMessage();

    const auto result = host.GetSession().InvokeInt32(*member.GetValue(), 7);
    EXPECT_TRUE(result.Succeeded()) << result.GetMessage();
    if (result.Succeeded())
        EXPECT_EQ(result.GetValue().value(), 12);

    auto nativeMember = host.GetSession().ResolveMember(*type.GetValue(), "AddSeven", parameterTypes, true);
    ASSERT_TRUE(nativeMember.Succeeded()) << nativeMember.GetMessage();
    const auto nativeResult = host.GetSession().InvokeInt32(*nativeMember.GetValue(), 7);
    EXPECT_TRUE(nativeResult.Succeeded()) << nativeResult.GetMessage();
    if (nativeResult.Succeeded())
        EXPECT_EQ(nativeResult.GetValue().value(), 14);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, BridgeExportExceptionsBecomeManagedStatuses)
{
    std::vector<std::string> loggedMessages;
    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .log = [&loggedMessages](std::uint32_t, const std::string&, const std::string& message) {
            loggedMessages.push_back(message);
        }
    };
    const auto status = host.Start(options);
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    const std::uint8_t invalidPath[]{ 0 };
    NwLoadAssemblyRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.path = { invalidPath, sizeof(invalidPath) };
    NwResult result{};
    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    const auto callStatus = api->LoadAssembly(&request, &result);

    EXPECT_EQ(callStatus, NwStatus_InternalError);
    EXPECT_EQ(result.status, NwStatus_InternalError);
    EXPECT_NE(result.error.message.data, nullptr);
    EXPECT_NE(result.error.managedTypeName.data, nullptr);
    EXPECT_FALSE(loggedMessages.empty());
    EXPECT_TRUE(api->ReleaseResult(&result) == NwStatus_Ok);
    EXPECT_EQ(result.structSize, 0u);
    EXPECT_EQ(api->ReleaseResult(&result), NwStatus_Ok);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, ManagedInvocationExceptionIncludesTypeAndMessage)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    const auto fixturePath = (
        std::filesystem::path(NEOWPF_DOTNET_BINDING_DIRECTORY) / "managed" / "BasicAssembly.dll").string();
    const auto assembly = host.GetSession().LoadAssembly(fixturePath);
    ASSERT_TRUE(assembly.Succeeded()) << assembly.GetMessage();
    const auto type = host.GetSession().ResolveType(
        "BasicAssembly.Domain.InvoiceService",
        *assembly.GetValue());
    ASSERT_TRUE(type.Succeeded()) << type.GetMessage();
    const auto int32Type = host.GetSession().ResolveType("System.Int32");
    ASSERT_TRUE(int32Type.Succeeded()) << int32Type.GetMessage();
    const std::array parameterTypes{ *int32Type.GetValue() };
    const auto member = host.GetSession().ResolveMember(
        *type.GetValue(),
        "ThrowManagedException",
        parameterTypes,
        true);
    ASSERT_TRUE(member.Succeeded()) << member.GetMessage();

    NwValue argument{};
    argument.kind = NwValueKind_Int32;
    NwInvokeRequest request{};
    request.header = { sizeof(request), NW_ABI_VERSION };
    request.member = {
        member.GetValue()->GetContextId(),
        member.GetValue()->GetIndex(),
        member.GetValue()->GetGeneration(),
        static_cast<std::uint32_t>(member.GetValue()->GetKind())
    };
    request.arguments = &argument;
    request.argumentCount = 1;
    request.policy = NwExecutionPolicy_Managed;
    NwResult result{};
    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    EXPECT_EQ(api->InvokeMember(&request, &result), NwStatus_ManagedException);
    ASSERT_NE(result.error.message.data, nullptr);
    ASSERT_NE(result.error.managedTypeName.data, nullptr);
    const std::string message(
        reinterpret_cast<const char*>(result.error.message.data),
        static_cast<std::size_t>(result.error.message.length));
    const std::string managedTypeName(
        reinterpret_cast<const char*>(result.error.managedTypeName.data),
        static_cast<std::size_t>(result.error.managedTypeName.length));
    EXPECT_EQ(message, "fixture failure: 0");
    EXPECT_EQ(managedTypeName, "System.InvalidOperationException");
    EXPECT_EQ(api->ReleaseResult(&result), NwStatus_Ok);
    EXPECT_EQ(result.structSize, 0u);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, AsyncOperationsReportSuccessFaultAndCancellation)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    const auto fixturePath = (
        std::filesystem::path(NEOWPF_DOTNET_BINDING_DIRECTORY) / "managed" / "BasicAssembly.dll").string();
    const auto assembly = host.GetSession().LoadAssembly(fixturePath);
    ASSERT_TRUE(assembly.Succeeded()) << assembly.GetMessage();
    const auto type = host.GetSession().ResolveType(
        "BasicAssembly.Domain.InvoiceService",
        *assembly.GetValue());
    ASSERT_TRUE(type.Succeeded()) << type.GetMessage();
    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    OperationCompletion successCompletion;
    NwStatus callStatus = NwStatus_InternalError;
    const auto successOperation = StartAsyncCall(
        host.GetSession(), *type.GetValue(), "AddAsync", 8,
        successCompletion, callStatus);
    ASSERT_EQ(callStatus, NwStatus_Ok);
    ASSERT_TRUE(successCompletion.Wait());
    EXPECT_EQ(successCompletion.state, 1u);
    EXPECT_EQ(api->RetainReference(successOperation), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseReference(successOperation), NwStatus_Ok);
    std::uint32_t operationState = 0;
    EXPECT_EQ(api->GetOperationState(successOperation, &operationState), NwStatus_Ok);
    EXPECT_EQ(operationState, 1u);
    NwResult successResult{};
    EXPECT_EQ(api->TakeOperationResult(successOperation, &successResult), NwStatus_Ok);
    EXPECT_EQ(successResult.value.kind, NwValueKind_Int32);
    EXPECT_EQ(successResult.value.payload.i64, 13);
    EXPECT_EQ(api->ReleaseResult(&successResult), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseOperation(successOperation), NwStatus_Ok);

    OperationCompletion faultCompletion;
    const auto faultOperation = StartAsyncCall(
        host.GetSession(), *type.GetValue(), "FailAsync", 4,
        faultCompletion, callStatus);
    ASSERT_EQ(callStatus, NwStatus_Ok);
    ASSERT_TRUE(faultCompletion.Wait());
    EXPECT_EQ(faultCompletion.state, 2u);
    NwResult faultResult{};
    EXPECT_EQ(api->TakeOperationResult(faultOperation, &faultResult), NwStatus_ManagedException);
    ASSERT_NE(faultResult.error.message.data, nullptr);
    EXPECT_EQ(api->ReleaseResult(&faultResult), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseOperation(faultOperation), NwStatus_Ok);

    OperationCompletion cancelCompletion;
    const auto cancelOperation = StartAsyncCall(
        host.GetSession(), *type.GetValue(), "WaitAsync", 3,
        cancelCompletion, callStatus);
    ASSERT_EQ(callStatus, NwStatus_Ok);
    EXPECT_EQ(api->CancelOperation(cancelOperation), NwStatus_Ok);
    ASSERT_TRUE(cancelCompletion.Wait());
    EXPECT_EQ(cancelCompletion.state, 3u);
    NwResult cancelResult{};
    EXPECT_EQ(api->TakeOperationResult(cancelOperation, &cancelResult), NwStatus_Canceled);
    EXPECT_EQ(api->ReleaseResult(&cancelResult), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseOperation(cancelOperation), NwStatus_Ok);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, LoadAssemblyRejectsTruncatedRequest)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    constexpr std::uint8_t missingAssembly[]{ 'm', 'i', 's', 's', 'i', 'n', 'g', '.', 'd', 'l', 'l' };
    NwLoadAssemblyRequest request{};
    request.header = {
        sizeof(request) - sizeof(request.reserved),
        NW_ABI_VERSION
    };
    request.path = { missingAssembly, sizeof(missingAssembly) };
    NwResult result{};
    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    const auto callStatus = api->LoadAssembly(&request, &result);

    EXPECT_EQ(callStatus, NwStatus_InvalidArgument);
    EXPECT_EQ(api->ReleaseResult(&result), NwStatus_Ok);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, RequestMajorVersionMismatchReturnsAbiMismatch)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    NwResolveTypeRequest request{};
    request.header = { sizeof(request), 2u << 16 };
    NwResult result{};
    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    const auto callStatus = api->ResolveType(&request, &result);

    EXPECT_EQ(callStatus, NwStatus_AbiMismatch);
    EXPECT_EQ(api->ReleaseResult(&result), NwStatus_Ok);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, MetadataReferencesAreIdempotentAndUnsupportedRequestsAreExplicit)
{
    NeoWPF::DotNet::Hosting::Host host;
    const auto status = host.Start();
    if (status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::RuntimeNotFound ||
        status.Code() == NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
    {
        GTEST_SKIP() << status.Message();
    }
    ASSERT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::Ok) << status.Message();

    const auto fixturePath = (
        std::filesystem::path(NEOWPF_DOTNET_BINDING_DIRECTORY) / "managed" / "BasicAssembly.dll").string();
    auto assembly = host.GetSession().LoadAssembly(fixturePath);
    ASSERT_TRUE(assembly.Succeeded()) << assembly.GetMessage();
    auto repeatedAssembly = host.GetSession().LoadAssembly(fixturePath);
    ASSERT_TRUE(repeatedAssembly.Succeeded()) << repeatedAssembly.GetMessage();
    EXPECT_EQ(assembly.GetValue()->GetIndex(), repeatedAssembly.GetValue()->GetIndex());

    auto type = host.GetSession().ResolveType("BasicAssembly.Domain.InvoiceService", *assembly.GetValue());
    ASSERT_TRUE(type.Succeeded()) << type.GetMessage();
    auto repeatedType = host.GetSession().ResolveType("BasicAssembly.Domain.InvoiceService", *assembly.GetValue());
    ASSERT_TRUE(repeatedType.Succeeded()) << repeatedType.GetMessage();
    EXPECT_EQ(type.GetValue()->GetIndex(), repeatedType.GetValue()->GetIndex());

    auto int32Type = host.GetSession().ResolveType("System.Int32");
    ASSERT_TRUE(int32Type.Succeeded()) << int32Type.GetMessage();
    const std::array parameterTypes{ *int32Type.GetValue() };
    auto member = host.GetSession().ResolveMember(*type.GetValue(), "AddTen", parameterTypes, true);
    ASSERT_TRUE(member.Succeeded()) << member.GetMessage();
    auto repeatedMember = host.GetSession().ResolveMember(*type.GetValue(), "AddTen", parameterTypes, true);
    ASSERT_TRUE(repeatedMember.Succeeded()) << repeatedMember.GetMessage();
    EXPECT_EQ(member.GetValue()->GetIndex(), repeatedMember.GetValue()->GetIndex());

    const auto* api = host.GetSession().GetBridgeApi();
    ASSERT_NE(api, nullptr);

    const NwReference assemblyReference{
        assembly.GetValue()->GetContextId(),
        assembly.GetValue()->GetIndex(),
        assembly.GetValue()->GetGeneration(),
        static_cast<std::uint32_t>(assembly.GetValue()->GetKind())
    };
    EXPECT_EQ(api->RetainReference(assemblyReference), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseReference(assemblyReference), NwStatus_Ok);

    const std::string typeName = "BasicAssembly.Domain.InvoiceService";
    const auto resolveAssemblyReference = [&](NwReference reference) {
        NwResolveTypeRequest request{};
        request.header = { sizeof(request), NW_ABI_VERSION };
        request.assembly = reference;
        request.fullName = {
            reinterpret_cast<const std::uint8_t*>(typeName.data()),
            typeName.size()
        };
        NwResult result{};
        const auto resolveStatus = api->ResolveType(&request, &result);
        EXPECT_EQ(api->ReleaseResult(&result), NwStatus_Ok);
        return resolveStatus;
    };
    auto wrongContextReference = assemblyReference;
    wrongContextReference.contextId++;
    EXPECT_EQ(resolveAssemblyReference(wrongContextReference), NwStatus_WrongContext);
    auto wrongKindReference = assemblyReference;
    wrongKindReference.kind = NwReferenceKind_Type;
    EXPECT_EQ(resolveAssemblyReference(wrongKindReference), NwStatus_WrongKind);
    auto staleReference = assemblyReference;
    staleReference.index = UINT32_MAX;
    EXPECT_EQ(resolveAssemblyReference(staleReference), NwStatus_StaleReference);

    NwResolveMemberRequest constructorRequest{};
    constructorRequest.header = { sizeof(constructorRequest), NW_ABI_VERSION };
    constructorRequest.type = {
        type.GetValue()->GetContextId(),
        type.GetValue()->GetIndex(),
        type.GetValue()->GetGeneration(),
        static_cast<std::uint32_t>(type.GetValue()->GetKind())
    };
    constructorRequest.memberKind = 0;
    NwResult constructorResult{};
    ASSERT_EQ(api->ResolveMember(&constructorRequest, &constructorResult), NwStatus_Ok);
    const auto constructorReference = constructorResult.value.payload.reference;
    EXPECT_EQ(api->ReleaseResult(&constructorResult), NwStatus_Ok);

    NwCreateObjectRequest createRequest{};
    createRequest.header = { sizeof(createRequest), NW_ABI_VERSION };
    createRequest.constructor = constructorReference;
    createRequest.policy = NwExecutionPolicy_Managed;
    NwResult objectResult{};
    ASSERT_EQ(api->CreateObject(&createRequest, &objectResult), NwStatus_Ok);
    const auto objectReference = objectResult.value.payload.reference;
    EXPECT_EQ(objectReference.kind, NwReferenceKind_Object);
    EXPECT_EQ(api->ReleaseResult(&objectResult), NwStatus_Ok);
    EXPECT_EQ(api->RetainReference(objectReference), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseReference(objectReference), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseReference(objectReference), NwStatus_Ok);
    EXPECT_EQ(api->ReleaseReference(objectReference), NwStatus_StaleReference);

    NwValue applicationArgument{};
    applicationArgument.kind = NwValueKind_Int32;
    applicationArgument.payload.i64 = 5;
    NwInvokeRequest applicationRequest{};
    applicationRequest.header = { sizeof(applicationRequest), NW_ABI_VERSION };
    applicationRequest.member = {
        member.GetValue()->GetContextId(),
        member.GetValue()->GetIndex(),
        member.GetValue()->GetGeneration(),
        static_cast<std::uint32_t>(member.GetValue()->GetKind())
    };
    applicationRequest.arguments = &applicationArgument;
    applicationRequest.argumentCount = 1;
    applicationRequest.policy = NwExecutionPolicy_Application;
    NwResult applicationResult{};
    EXPECT_EQ(api->InvokeMember(&applicationRequest, &applicationResult), NwStatus_Unsupported);
    EXPECT_EQ(api->ReleaseResult(&applicationResult), NwStatus_Ok);

    NwResolveTypeRequest arrayRequest{};
    arrayRequest.header = { sizeof(arrayRequest), NW_ABI_VERSION };
    arrayRequest.fullName = {
        reinterpret_cast<const std::uint8_t*>("System.Int32"),
        sizeof("System.Int32") - 1
    };
    arrayRequest.shape = 1;
    arrayRequest.arrayRank = 1;
    NwResult arrayResult{};
    EXPECT_EQ(api->ResolveType(&arrayRequest, &arrayResult), NwStatus_Unsupported);
    EXPECT_EQ(api->ReleaseResult(&arrayResult), NwStatus_Ok);

    const std::string overloadedName = "Overloaded";
    NwResolveMemberRequest ambiguousRequest{};
    ambiguousRequest.header = { sizeof(ambiguousRequest), NW_ABI_VERSION };
    ambiguousRequest.type = {
        type.GetValue()->GetContextId(),
        type.GetValue()->GetIndex(),
        type.GetValue()->GetGeneration(),
        static_cast<std::uint32_t>(type.GetValue()->GetKind())
    };
    ambiguousRequest.name = {
        reinterpret_cast<const std::uint8_t*>(overloadedName.data()),
        overloadedName.size()
    };
    ambiguousRequest.memberKind = 1;
    ambiguousRequest.flags = 1 | 2;
    NwResult ambiguousResult{};
    const auto resolveStatus = api->ResolveMember(&ambiguousRequest, &ambiguousResult);
    const auto resolveMessage = ambiguousResult.error.message.data
        ? std::string(reinterpret_cast<const char*>(ambiguousResult.error.message.data),
            static_cast<std::size_t>(ambiguousResult.error.message.length))
        : std::string();
    EXPECT_EQ(resolveStatus, NwStatus_Ambiguous) << resolveMessage;
    EXPECT_NE(ambiguousResult.error.message.data, nullptr);
    EXPECT_GT(ambiguousResult.error.message.length, 0u);
    EXPECT_EQ(api->ReleaseResult(&ambiguousResult), NwStatus_Ok);
    EXPECT_TRUE(host.Stop());
}

TEST(DotNetHostTests, HostStartReportsMissingFrameworkWithRawHostFxrResult)
{
    const auto hostfxrPath = NeoWPF::DotNet::Hosting::Host::ResolveHostFxrPath({});
    if (hostfxrPath.empty())
        GTEST_SKIP() << "No hostfxr is installed.";

    const auto bridgeDirectory = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.missing-framework";
    CreateBridgeFiles(bridgeDirectory);
    {
        std::ofstream runtimeConfig(bridgeDirectory / "NeoWPF.DotNet.Bridge.runtimeconfig.json");
        runtimeConfig << R"({"runtimeOptions":{"tfm":"net99.0","framework":{"name":"Microsoft.NETCore.App","version":"99.0.0","rollForward":"Disable"}}})";
    }

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath,
        .bridgeDirectory = bridgeDirectory
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::FrameworkMissing)
        << status.Message() << " (hostfxr result " << status.GetRawHostFxrResult() << ")";
    EXPECT_NE(status.GetRawHostFxrResult(), 0);
    EXPECT_FALSE(host.Stop());
    std::filesystem::remove_all(bridgeDirectory);
}

TEST(DotNetHostTests, HostStartReportsMissingHostFxrExports)
{
#if defined(_WIN32)
    const auto hostfxrPath = std::filesystem::path("C:/Windows/System32/kernel32.dll");
#elif defined(__linux__)
    const auto hostfxrPath = std::filesystem::path("/lib/x86_64-linux-gnu/libm.so.6");
#else
    GTEST_SKIP() << "No portable system library is available for this platform.";
#endif
    if (!std::filesystem::exists(hostfxrPath))
        GTEST_SKIP() << "The system library is not available on this system.";

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrSymbolMissing)
        << status.Message();
    EXPECT_FALSE(status.Succeeded());
    EXPECT_FALSE(host.Stop());
}

TEST(DotNetHostTests, HostStartDoesNotSucceedFromPathResolutionAlone)
{
    const auto hostfxrPath = std::filesystem::temp_directory_path() /
        "NeoWPF.DotNetHostTests.fake-hostfxr";
    std::ofstream(hostfxrPath).put(' ');

    NeoWPF::DotNet::Hosting::Host host;
    const NeoWPF::DotNet::Hosting::HostOptions options{
        .hostfxrPath = hostfxrPath
    };
    const auto status = host.Start(options);

    EXPECT_EQ(status.Code(), NeoWPF::DotNet::Hosting::HostStatusCode::HostFxrLoadFailed);
    EXPECT_FALSE(status.Succeeded());
    EXPECT_FALSE(host.Stop());
    std::filesystem::remove(hostfxrPath);
}

TEST(DotNetHostTests, ManagedBridgeResolvesExecutableRelativeDirectories)
{
    const auto executableDirectory = std::filesystem::temp_directory_path() /
        ("NeoWPF.DotNetHostTests." + std::to_string(std::rand()));
    const auto bridgeDirectory = executableDirectory / "NeoWPF.DotNet";

    CreateBridgeFiles(bridgeDirectory);
    const auto resolved = NeoWPF::DotNet::Hosting::Host::ResolveManagedBridgeDirectory(
        executableDirectory,
        std::filesystem::path());

    EXPECT_EQ(resolved, bridgeDirectory);
    std::filesystem::remove_all(executableDirectory);
}

TEST(DotNetHostTests, ManagedBridgeFallsBackToExecutableDirectory)
{
    const auto executableDirectory = std::filesystem::temp_directory_path() /
        ("NeoWPF.DotNetHostTests." + std::to_string(std::rand()));
    CreateBridgeFiles(executableDirectory);

    const auto resolved = NeoWPF::DotNet::Hosting::Host::ResolveManagedBridgeDirectory(
        executableDirectory,
        std::filesystem::path());

    EXPECT_EQ(resolved, executableDirectory);
    std::filesystem::remove_all(executableDirectory);
}

}
