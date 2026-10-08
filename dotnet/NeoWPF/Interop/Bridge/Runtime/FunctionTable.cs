using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class FunctionTable
{
    private const uint AbiVersion = 1u << 16;
    private static NwBridgeApi* s_api;

    public static NwBridgeApi* Get()
    {
        if (s_api is not null)
            return s_api;

        s_api = (NwBridgeApi*)NativeMemory.AllocZeroed((nuint)sizeof(NwBridgeApi));
        if (s_api is null)
            return null;

        s_api->StructSize = (uint)sizeof(NwBridgeApi);
        s_api->AbiVersion = AbiVersion;
        s_api->Shutdown = (nint)(delegate* unmanaged[Cdecl]<uint, int>)&Shutdown;
        return s_api;
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int Shutdown(uint drainTimeoutMilliseconds) => 0;
}