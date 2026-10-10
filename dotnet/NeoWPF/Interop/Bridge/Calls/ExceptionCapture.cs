namespace NeoWPF.DotNet.Bridge.Calls;

using NeoWPF.DotNet.Bridge.Operations;
using NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class ExceptionCapture
{
    public static int RunEntryPoint(
        NwBridgeInitInfo* info,
        NwBridgeApi** api,
        delegate* managed<NwBridgeInitInfo*, NwBridgeApi**, int> operation)
    {
        if (api is not null)
            *api = null;

        try
        {
            var status = operation(info, api);
            if (status != AbiConstants.Ok && api is not null)
                *api = null;
            return status;
        }
        catch (OutOfMemoryException)
        {
            ResetSession();
            if (api is not null)
                *api = null;
            return AbiConstants.OutOfMemory;
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            ResetSession();
            if (api is not null)
                *api = null;
            return AbiConstants.InternalError;
        }
    }

    public static int Run<TRequest>(
        TRequest* request,
        NwResult* result,
        delegate* managed<TRequest*, NwResult*, int> operation)
        where TRequest : unmanaged
    {
        if (!ResultWriter.Initialize(result))
            return AbiConstants.InvalidArgument;

        try
        {
            return operation(request, result);
        }
        catch (OutOfMemoryException)
        {
            return ResultWriter.FailWithoutAllocation(result, AbiConstants.OutOfMemory);
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return ResultWriter.Fail(
                result,
                AbiConstants.InternalError,
                exception.Message,
                exception.GetType().FullName,
                exception.StackTrace);
        }
    }

    public static int RunResult(
        NwResult* result,
        delegate* managed<NwResult*, int> operation)
    {
        try
        {
            return operation(result);
        }
        catch (OutOfMemoryException)
        {
            return ResultWriter.FailWithoutAllocation(result, AbiConstants.OutOfMemory);
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return ResultWriter.FailWithoutAllocation(result, AbiConstants.InternalError);
        }
    }

    public static int Run(
        uint value,
        delegate* managed<uint, int> operation)
    {
        try
        {
            return operation(value);
        }
        catch (OutOfMemoryException)
        {
            return AbiConstants.OutOfMemory;
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return AbiConstants.InternalError;
        }
    }

    public static int Run(
        NwReference reference,
        delegate* managed<NwReference, int> operation)
    {
        try
        {
            return operation(reference);
        }
        catch (OutOfMemoryException)
        {
            return AbiConstants.OutOfMemory;
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return AbiConstants.InternalError;
        }
    }

    public static int Run(
        NwReference reference,
        uint* state,
        delegate* managed<NwReference, uint*, int> operation)
    {
        try
        {
            return operation(reference, state);
        }
        catch (OutOfMemoryException)
        {
            return AbiConstants.OutOfMemory;
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return AbiConstants.InternalError;
        }
    }

    public static int Run(
        NwReference reference,
        NwResult* result,
        delegate* managed<NwReference, NwResult*, int> operation)
    {
        if (!ResultWriter.Initialize(result))
            return AbiConstants.InvalidArgument;

        try
        {
            return operation(reference, result);
        }
        catch (OutOfMemoryException)
        {
            return ResultWriter.FailWithoutAllocation(result, AbiConstants.OutOfMemory);
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return ResultWriter.Fail(
                result,
                AbiConstants.InternalError,
                exception.Message,
                exception.GetType().FullName,
                exception.StackTrace);
        }
    }

    public static int Run(
        void* value,
        delegate* managed<void*, int> operation)
    {
        try
        {
            return operation(value);
        }
        catch (OutOfMemoryException)
        {
            return AbiConstants.OutOfMemory;
        }
        catch (Exception exception)
        {
            LogFailure(exception);
            return AbiConstants.InternalError;
        }
    }

    private static void LogFailure(Exception exception)
    {
        try
        {
            BridgeState.GetCurrent()?.Log(3, "NeoWPF.DotNet.Bridge", exception.ToString());
        }
        catch
        {
        }
    }

    private static void ResetSession()
    {
        try
        {
            BridgeState.GetCurrent()?.Shutdown();
        }
        catch
        {
        }
    }
}