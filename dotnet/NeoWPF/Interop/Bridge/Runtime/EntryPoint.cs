using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace NeoWPF.DotNet.Bridge.Runtime;

public static unsafe class EntryPoint
{
    private const uint AbiMajor = 1;
    private const int Ok = 0;
    private const int InvalidArgument = 1;
    private const int AbiMismatch = 2;
    private const int InternalError = 18;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int NwBridgeInitialize(NwBridgeInitInfo* info, NwBridgeApi** api)
    {
        if (info is null || api is null || info->ContextId == 0 ||
            info->StructSize < sizeof(NwBridgeInitInfo))
        {
            return InvalidArgument;
        }

        if (info->AbiVersion >> 16 != AbiMajor || !AbiLayouts.Validate())
            return AbiMismatch;

        BridgeState.Initialize(info->ContextId);
        var functionTable = FunctionTable.Get();
        if (functionTable is null || functionTable->LoadAssembly == 0 ||
            functionTable->ResolveType == 0 || functionTable->ResolveMember == 0 ||
            functionTable->InvokeMember == 0 || functionTable->VerifyContract == 0 ||
            functionTable->ReleaseResult == 0 || functionTable->Shutdown == 0)
            return InternalError;

        *api = functionTable;
        return Ok;
    }
}