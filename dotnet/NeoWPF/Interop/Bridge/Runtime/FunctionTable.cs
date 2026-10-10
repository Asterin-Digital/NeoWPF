using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using NeoWPF.DotNet.Bridge.Calls;
using NeoWPF.DotNet.Bridge.Loading;
using NeoWPF.DotNet.Bridge.Operations;
using NeoWPF.DotNet.Bridge.Types;
using NeoWPF.DotNet.Bridge.References;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class FunctionTable
{
    private static readonly object s_gate = new();
    private static NwBridgeApi* s_api;

    public static NwBridgeApi* Get()
    {
        if (s_api is not null)
            return s_api;

        lock (s_gate)
        {
            if (s_api is not null)
                return s_api;

            var api = (NwBridgeApi*)NativeMemory.AllocZeroed((nuint)sizeof(NwBridgeApi));
            api->StructSize = (uint)sizeof(NwBridgeApi);
            api->AbiVersion = AbiVersion;
            api->LoadAssembly = (nint)(delegate* unmanaged[Cdecl]<NwLoadAssemblyRequest*, NwResult*, int>)&LoadAssembly;
            api->ResolveType = (nint)(delegate* unmanaged[Cdecl]<NwResolveTypeRequest*, NwResult*, int>)&ResolveType;
            api->ResolveMember = (nint)(delegate* unmanaged[Cdecl]<NwResolveMemberRequest*, NwResult*, int>)&ResolveMember;
            api->CreateObject = (nint)(delegate* unmanaged[Cdecl]<NwCreateObjectRequest*, NwResult*, int>)&CreateObject;
            api->GetOperationState = (nint)(delegate* unmanaged[Cdecl]<NwReference, uint*, int>)&GetOperationState;
            api->TakeOperationResult = (nint)(delegate* unmanaged[Cdecl]<NwReference, NwResult*, int>)&TakeOperationResult;
            api->CancelOperation = (nint)(delegate* unmanaged[Cdecl]<NwReference, int>)&CancelOperation;
            api->ReleaseOperation = (nint)(delegate* unmanaged[Cdecl]<NwReference, int>)&ReleaseOperation;
            api->RetainReference = (nint)(delegate* unmanaged[Cdecl]<NwReference, int>)&RetainReference;
            api->ReleaseReference = (nint)(delegate* unmanaged[Cdecl]<NwReference, int>)&ReleaseReference;
            api->InvokeMember = (nint)(delegate* unmanaged[Cdecl]<NwInvokeRequest*, NwResult*, int>)&InvokeMember;
            api->VerifyContract = (nint)(delegate* unmanaged[Cdecl]<NwVerifyContractRequest*, NwResult*, int>)&VerifyContract;
            api->ReleaseResult = (nint)(delegate* unmanaged[Cdecl]<NwResult*, int>)&ReleaseResult;
            api->ReleaseBuffer = (nint)(delegate* unmanaged[Cdecl]<void*, int>)&ReleaseBuffer;
            api->Shutdown = (nint)(delegate* unmanaged[Cdecl]<uint, int>)&Shutdown;
            s_api = api;
            return api;
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int LoadAssembly(NwLoadAssemblyRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &AssemblyRegistry.Load);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ResolveType(NwResolveTypeRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &MemberResolution.ResolveType);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ResolveMember(NwResolveMemberRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &MemberResolution.ResolveMember);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int CreateObject(NwCreateObjectRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &ObjectConstruction.Create);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int GetOperationState(NwReference reference, uint* state) =>
        ExceptionCapture.Run(reference, state, &OperationTracking.GetState);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int TakeOperationResult(NwReference reference, NwResult* result) =>
        ExceptionCapture.Run(reference, result, &OperationTracking.TakeResult);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int CancelOperation(NwReference reference) =>
        ExceptionCapture.Run(reference, &OperationTracking.Cancel);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseOperation(NwReference reference) =>
        ExceptionCapture.Run(reference, &OperationTracking.Release);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int RetainReference(NwReference reference) =>
        ExceptionCapture.Run(reference, &ReferenceOperations.Retain);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseReference(NwReference reference) =>
        ExceptionCapture.Run(reference, &ReferenceOperations.Release);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int InvokeMember(NwInvokeRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &MemberInvocation.InvokeInt32);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int VerifyContract(NwVerifyContractRequest* request, NwResult* result) =>
        ExceptionCapture.Run(request, result, &ContractVerification.Verify);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseResult(NwResult* result) =>
        ExceptionCapture.RunResult(result, &ResultWriter.Release);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseBuffer(void* buffer) =>
        ExceptionCapture.Run(buffer, &ResultWriter.ReleaseBuffer);

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int Shutdown(uint drainTimeoutMilliseconds) =>
        ExceptionCapture.Run(drainTimeoutMilliseconds, &ShutdownCore);

    private static int ShutdownCore(uint drainTimeoutMilliseconds)
    {
        _ = drainTimeoutMilliseconds;
        BridgeState.GetCurrent()?.Shutdown();
        return Ok;
    }
}