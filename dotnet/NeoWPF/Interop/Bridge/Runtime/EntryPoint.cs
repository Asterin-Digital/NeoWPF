using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using NeoWPF.DotNet.Bridge.Calls;

namespace NeoWPF.DotNet.Bridge.Runtime;

using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

public static unsafe class EntryPoint
{
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int NwBridgeInitialize(NwBridgeInitInfo* info, NwBridgeApi** api) =>
        ExceptionCapture.RunEntryPoint(info, api, &Initialize);

    private static int Initialize(NwBridgeInitInfo* info, NwBridgeApi** api)
    {
        if (info is null || api is null || info->ContextId == 0 ||
            info->StructSize < sizeof(NwBridgeInitInfo))
        {
            return InvalidArgument;
        }

        if (info->AbiVersion >> 16 != AbiMajor || !AbiLayouts.Validate())
            return AbiMismatch;

        var hostCallbacks = default(NwHostCallbacks);
        if (info->Host != 0)
        {
            var callbacks = (NwHostCallbacks*)info->Host;
            if (callbacks->AbiVersion >> 16 != AbiMajor)
                return AbiMismatch;
            if (callbacks->StructSize < sizeof(NwHostCallbacks))
                return InvalidArgument;
            hostCallbacks = *callbacks;
        }

        BridgeState.Initialize(info->ContextId, hostCallbacks);
        var functionTable = FunctionTable.Get();
        if (functionTable is null || functionTable->LoadAssembly == 0 ||
            functionTable->ResolveType == 0 || functionTable->ResolveMember == 0 ||
            functionTable->InvokeMember == 0 || functionTable->VerifyContract == 0 ||
            functionTable->ReleaseResult == 0 || functionTable->Shutdown == 0)
        {
            BridgeState.GetCurrent()?.Shutdown();
            return InternalError;
        }

        *api = functionTable;
        return Ok;
    }
}