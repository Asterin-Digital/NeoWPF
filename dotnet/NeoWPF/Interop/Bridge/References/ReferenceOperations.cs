using NeoWPF.DotNet.Bridge.Runtime;
using NeoWPF.DotNet.Bridge.Operations;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.References;

internal static class ReferenceOperations
{
    public static int Retain(NwReference reference) => GetMetadataStatus(reference, retain: true);

    public static int Release(NwReference reference) => GetMetadataStatus(reference, retain: false);

    private static int GetMetadataStatus(NwReference reference, bool retain)
    {
        var state = BridgeState.GetCurrent();
        if (state is null)
            return SessionEnded;
        if (reference.Kind is not (AssemblyReferenceKind or TypeReferenceKind or MemberReferenceKind or
            ObjectReferenceKind or OperationReferenceKind))
            return WrongKind;

        if (!retain && reference.Kind == OperationReferenceKind)
            return OperationTracking.Release(reference);

        return retain
            ? state.References.Retain(reference)
            : state.References.Release(reference);
    }
}