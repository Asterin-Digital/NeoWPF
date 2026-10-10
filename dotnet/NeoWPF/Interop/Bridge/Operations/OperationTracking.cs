using NeoWPF.DotNet.Bridge.References;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Operations;

internal static unsafe class OperationTracking
{
    public static int GetState(NwReference reference, uint* state)
    {
        if (state is null)
            return InvalidArgument;
        var recordStatus = GetRecord(reference, out var record);
        if (recordStatus != Ok)
            return recordStatus;
        *state = record!.State;
        return Ok;
    }

    public static int TakeResult(NwReference reference, NwResult* result)
    {
        var recordStatus = GetRecord(reference, out var record);
        if (recordStatus != Ok)
            return ResultWriter.Fail(result, recordStatus);

        var status = record!.TakeResult(out var value, out var exception);
        if (status == Ok)
        {
            ResultWriter.SetInt32(result, value);
            return Ok;
        }
        if (status == ManagedException)
        {
            return ResultWriter.Fail(
                result,
                status,
                exception?.Message,
                exception?.GetType().FullName,
                exception?.StackTrace);
        }
        return ResultWriter.Fail(result, status);
    }

    public static int Cancel(NwReference reference)
    {
        var recordStatus = GetRecord(reference, out var record);
        return recordStatus == Ok ? record!.Cancel() : recordStatus;
    }

    public static int Release(NwReference reference)
    {
        var recordStatus = GetRecord(reference, out var record);
        if (recordStatus != Ok)
            return recordStatus;

        var state = BridgeState.GetCurrent();
        if (state is null)
            return SessionEnded;
        var status = state.References.Release(reference, out var fullyReleased);
        if (status == Ok && fullyReleased)
            record!.Release();
        return status;
    }

    private static int GetRecord(NwReference reference, out OperationRecord? record)
    {
        record = null;
        var state = BridgeState.GetCurrent();
        if (state is null)
            return SessionEnded;
        var status = state.References.GetStatus(reference, OperationReferenceKind);
        if (status != Ok)
            return status;
        return state.References.TryGet(reference, OperationReferenceKind, out record)
            ? Ok
            : WrongKind;
    }
}