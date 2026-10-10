using System.Runtime.InteropServices;

namespace NeoWPF.DotNet.Bridge.Runtime;

[StructLayout(LayoutKind.Sequential)]
internal struct NwReference
{
    public uint ContextId;
    public uint Index;
    public uint Generation;
    public uint Kind;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwStringView
{
    public nint Data;
    public ulong Length;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwByteView
{
    public nint Data;
    public ulong Length;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwArrayView
{
    public nint Data;
    public ulong Count;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwDecimal
{
    public int Low;
    public int Middle;
    public int High;
    public int Flags;
}

[StructLayout(LayoutKind.Explicit, Size = 16)]
internal struct NwValuePayload
{
    [FieldOffset(0)] public byte Boolean;
    [FieldOffset(0)] public long Signed;
    [FieldOffset(0)] public ulong Unsigned;
    [FieldOffset(0)] public float Float32;
    [FieldOffset(0)] public double Float64;
    [FieldOffset(0)] public NwDecimal Decimal;
    [FieldOffset(0)] public NwStringView String;
    [FieldOffset(0)] public NwByteView Bytes;
    [FieldOffset(0)] public NwArrayView Array;
    [FieldOffset(0)] public NwReference Reference;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwValue
{
    public uint Kind;
    public uint Flags;
    public NwValuePayload Payload;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwError
{
    public uint StructSize;
    public uint Code;
    public NwStringView Message;
    public NwStringView ManagedTypeName;
    public NwStringView StackTrace;
    public NwReference Exception;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwResult
{
    public uint StructSize;
    public uint AbiVersion;
    public int Status;
    public uint Reserved;
    public NwValue Value;
    public NwError Error;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwRequestHeader
{
    public uint StructSize;
    public uint AbiVersion;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwLoadAssemblyRequest
{
    public NwRequestHeader Header;
    public NwStringView Path;
    public NwStringView LoadContextName;
    public uint Flags;
    public uint Reserved;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwResolveTypeRequest
{
    public NwRequestHeader Header;
    public NwReference Assembly;
    public NwStringView FullName;
    public nint TypeArguments;
    public uint TypeArgumentCount;
    public uint Shape;
    public uint ArrayRank;
    public uint Reserved;
    public NwReference ElementType;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwParameterDescriptor
{
    public NwReference Type;
    public uint Passing;
    public uint Reserved;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwResolveMemberRequest
{
    public NwRequestHeader Header;
    public NwReference Type;
    public NwStringView Name;
    public uint MemberKind;
    public uint Flags;
    public uint GenericArity;
    public uint ParameterCount;
    public nint Parameters;
    public nint GenericArguments;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwInvokeRequest
{
    public NwRequestHeader Header;
    public NwReference Member;
    public NwReference Target;
    public nint Arguments;
    public uint ArgumentCount;
    public uint Policy;
    public uint Flags;
    public uint Reserved;
    public nint Sink;
    public nint Outputs;
    public uint OutputCount;
    public uint Reserved2;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwVerifyContractRequest
{
    public NwRequestHeader Header;
    public NwStringView BindingName;
    public nint Fingerprint;
    public uint AssemblyCount;
    public uint Reserved;
    public nint AssemblyMvids;
}

[StructLayout(LayoutKind.Sequential)]
public struct NwBridgeApi
{
    public uint StructSize;
    public uint AbiVersion;
    public nint LoadAssembly;
    public nint ResolveType;
    public nint ResolveMember;
    public nint CreateObject;
    public nint InvokeMember;
    public nint GetProperty;
    public nint SetProperty;
    public nint RegisterCallback;
    public nint UnregisterCallback;
    public nint RetainReference;
    public nint ReleaseReference;
    public nint GetOperationState;
    public nint TakeOperationResult;
    public nint CancelOperation;
    public nint ReleaseOperation;
    public nint DescribeReference;
    public nint VerifyContract;
    public nint ReleaseResult;
    public nint ReleaseBuffer;
    public nint Shutdown;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NwHostCallbacks
{
    public uint StructSize;
    public uint AbiVersion;
    public nint UserData;
    public nint Log;
}

[StructLayout(LayoutKind.Sequential)]
public struct NwBridgeInitInfo
{
    public uint StructSize;
    public uint AbiVersion;
    public uint ContextId;
    public uint Flags;
    public nint Host;
}

internal static class AbiLayouts
{
    public static bool Validate() =>
        Marshal.SizeOf<NwReference>() == 16 &&
        Marshal.OffsetOf<NwReference>(nameof(NwReference.Kind)).ToInt32() == 12 &&
        Marshal.SizeOf<NwStringView>() == 16 &&
        Marshal.OffsetOf<NwStringView>(nameof(NwStringView.Length)).ToInt32() == 8 &&
        Marshal.SizeOf<NwByteView>() == 16 &&
        Marshal.SizeOf<NwArrayView>() == 16 &&
        Marshal.SizeOf<NwDecimal>() == 16 &&
        Marshal.OffsetOf<NwDecimal>(nameof(NwDecimal.Flags)).ToInt32() == 12 &&
        Marshal.SizeOf<NwValue>() == 24 &&
        Marshal.OffsetOf<NwValue>(nameof(NwValue.Payload)).ToInt32() == 8 &&
        Marshal.SizeOf<NwBridgeApi>() == 168 &&
        Marshal.SizeOf<NwRequestHeader>() == 8 &&
        Marshal.SizeOf<NwLoadAssemblyRequest>() == 48 &&
        Marshal.SizeOf<NwResolveTypeRequest>() == 80 &&
        Marshal.SizeOf<NwParameterDescriptor>() == 24 &&
        Marshal.SizeOf<NwResolveMemberRequest>() == 72 &&
        Marshal.SizeOf<NwInvokeRequest>() == 88 &&
        Marshal.SizeOf<NwVerifyContractRequest>() == 48 &&
        Marshal.SizeOf<NwBridgeInitInfo>() == 24;
}