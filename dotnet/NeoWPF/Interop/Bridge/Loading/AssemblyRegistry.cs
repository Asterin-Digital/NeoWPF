using System.Reflection;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Loading;

internal static unsafe class AssemblyRegistry
{
    public static int Load(NwLoadAssemblyRequest* request, NwResult* result)
    {
        if (request is null)
            return ResultWriter.Fail(result, InvalidArgument);
        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwLoadAssemblyRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);
        if (request->Reserved != 0 || (request->Flags & ~1u) != 0)
            return ResultWriter.Fail(result, InvalidArgument);

        if (request->LoadContextName.Length != 0 && AbiString.Read(request->LoadContextName) != "Isolated")
            return ResultWriter.Fail(result, Unsupported);

        var path = AbiString.Read(request->Path);
        if (string.IsNullOrWhiteSpace(path))
            return ResultWriter.Fail(result, InvalidArgument);

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);

        Assembly assembly;
        try
        {
            assembly = state.LoadAssembly(path);
        }
        catch (FileNotFoundException exception)
        {
            return ResultWriter.Fail(result, NotFound, exception.Message);
        }

        ResultWriter.SetReference(result, state.References.Add(assembly, AssemblyReferenceKind));
        return Ok;
    }
}