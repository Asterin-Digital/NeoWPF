using System.Reflection;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Calls;

internal static unsafe class ObjectConstruction
{
    public static int Create(NwCreateObjectRequest* request, NwResult* result)
    {
        if (request is null)
            return ResultWriter.Fail(result, InvalidArgument);

        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwCreateObjectRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);
        if (request->ArgumentCount != 0 || request->Arguments != 0)
            return ResultWriter.Fail(result, Unsupported);
        if (request->Policy > BackgroundPolicy)
            return ResultWriter.Fail(result, InvalidArgument);
        if (request->Policy is ApplicationPolicy or BackgroundPolicy)
            return ResultWriter.Fail(result, Unsupported);

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);

        var constructorStatus = state.References.TryAcquire<ConstructorInfo>(
            request->Constructor,
            MemberReferenceKind,
            out var constructor,
            out var constructorLease);
        if (constructorStatus != Ok)
            return ResultWriter.Fail(result, constructorStatus);
        using var lease = constructorLease!;
        if (constructor!.GetParameters().Length != 0)
            return ResultWriter.Fail(result, Unsupported);

        try
        {
            var instance = constructor.Invoke([]);
            ResultWriter.SetReference(result, state.References.Add(instance, ObjectReferenceKind));
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
}