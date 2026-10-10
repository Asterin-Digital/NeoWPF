using System.Reflection;
using NeoWPF.DotNet.Bridge.Operations;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Calls;

internal static unsafe class MemberInvocation
{
    public static int InvokeInt32(NwInvokeRequest* request, NwResult* result)
    {
        if (request is null)
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }
        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwInvokeRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);

        if (request->ArgumentCount != 1 || request->Arguments == 0 ||
            request->Reserved != 0 || request->Reserved2 != 0 ||
            (request->Flags & ~3u) != 0)
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }
        if (request->Outputs != 0 || request->OutputCount != 0)
            return ResultWriter.Fail(result, Unsupported);
        if (request->Policy > BackgroundPolicy)
            return ResultWriter.Fail(result, InvalidArgument);
        if (request->Policy is ApplicationPolicy or BackgroundPolicy)
            return ResultWriter.Fail(result, Unsupported);

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);

        var memberStatus = state.References.TryAcquire<MethodInfo>(
            request->Member,
            MemberReferenceKind,
            out var method,
            out var memberLease);
        if (memberStatus != Ok)
            return ResultWriter.Fail(result, memberStatus);
        using var lease = memberLease!;
        if (!method!.IsStatic)
            return ResultWriter.Fail(result, Unsupported);
        if (!RequestValidation.IsNull(request->Target))
            return ResultWriter.Fail(result, InvalidArgument);

        var arguments = (NwValue*)request->Arguments;
        var parameters = method.GetParameters();
        var hasCancellationToken = parameters is
            [{ ParameterType: var firstParameter }, { ParameterType: var secondParameter }] &&
            firstParameter == typeof(int) && secondParameter == typeof(CancellationToken);
        if (arguments[0].Kind != Int32ValueKind || arguments[0].Flags != 0 ||
            parameters.Length is not (1 or 2) || parameters[0].ParameterType != typeof(int) ||
            (parameters.Length == 2 && !hasCancellationToken) ||
            (method.ReturnType != typeof(int) && method.ReturnType != typeof(Task<int>)))
        {
            return ResultWriter.Fail(result, ConversionFailed);
        }

        int value;
        try
        {
            value = checked((int)arguments[0].Payload.Signed);
        }
        catch (OverflowException)
        {
            return ResultWriter.Fail(result, ConversionFailed);
        }

        if (method.ReturnType == typeof(Task<int>))
            return InvokeAsync(method, value, hasCancellationToken, request->Sink, state, result);
        if (request->Sink != 0)
            return ResultWriter.Fail(result, InvalidArgument);

        try
        {
            var returnedValue = method.Invoke(null, [value]);
            if (returnedValue is not int convertedValue)
                return ResultWriter.Fail(result, ConversionFailed);
            ResultWriter.SetInt32(result, convertedValue);
            return Ok;
        }
        catch (TargetInvocationException exception)
        {
            var managedException = exception.InnerException ?? exception;
            return ResultWriter.Fail(
                result,
                ManagedException,
                managedException.Message,
                managedException.GetType().FullName,
                managedException.StackTrace);
        }
    }

    private static int InvokeAsync(
        MethodInfo method,
        int value,
        bool hasCancellationToken,
        nint sinkAddress,
        BridgeState state,
        NwResult* result)
    {
        if (sinkAddress == 0)
            return ResultWriter.Fail(result, InvalidArgument);

        var sink = (NwOperationSink*)sinkAddress;
        if (sink->Completed == 0)
            return ResultWriter.Fail(result, InvalidArgument);

        var cancellation = hasCancellationToken ? new CancellationTokenSource() : null;
        try
        {
            object?[] invokeArguments = cancellation is null
                ? [value]
                : [value, cancellation.Token];
            if (method.Invoke(null, invokeArguments) is not Task<int> task)
            {
                cancellation?.Dispose();
                return ResultWriter.Fail(result, ConversionFailed);
            }

            var operation = new OperationRecord(
                task,
                cancellation,
                sink->UserData,
                (delegate* unmanaged[Cdecl]<nint, NwReference, uint, void>)sink->Completed);
            var reference = state.References.Add(operation, OperationReferenceKind);
            operation.Reference = reference;
            ResultWriter.SetReference(result, reference);
            operation.Start();
            return Ok;
        }
        catch (TargetInvocationException exception)
        {
            cancellation?.Dispose();
            var managedException = exception.InnerException ?? exception;
            return ResultWriter.Fail(
                result,
                ManagedException,
                managedException.Message,
                managedException.GetType().FullName,
                managedException.StackTrace);
        }
    }
}