using System.Reflection;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Loading;

internal static unsafe class ContractVerification
{
    public static int Verify(NwVerifyContractRequest* request, NwResult* result)
    {
        if (request is null)
            return ResultWriter.Fail(result, InvalidArgument);
        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwVerifyContractRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);
        if (request->Fingerprint == 0 || request->AssemblyCount == 0 ||
            request->AssemblyMvids == 0)
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);

        var bindingName = AbiString.Read(request->BindingName);
        var contractTypeName = $"{bindingName}.BindingContract";
        Type? contractType = null;
        foreach (var assembly in state.GetLoadedAssemblies())
        {
            contractType = assembly.GetType(contractTypeName, throwOnError: false, ignoreCase: false);
            if (contractType is not null)
                break;
        }
        if (contractType is null)
            return ResultWriter.Fail(result, ContractMismatch, $"Binding contract '{contractTypeName}' was not loaded.");

        var fingerprintField = contractType.GetField(
            "ContractFingerprint",
            BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
        if (fingerprintField?.GetValue(null) is not string contractFingerprint)
            return ResultWriter.Fail(result, ContractMismatch, "The binding contract does not expose its fingerprint.");

        var assemblyNamesField = contractType.GetField(
            "AssemblyNames",
            BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
        var assemblyMvidsField = contractType.GetField(
            "AssemblyMvids",
            BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly);
        if (assemblyNamesField?.GetValue(null) is not string[] assemblyNames ||
            assemblyMvidsField?.GetValue(null) is not string[] contractMvidTexts ||
            assemblyNames.Length != contractMvidTexts.Length ||
            request->AssemblyCount != assemblyNames.Length)
        {
            return ResultWriter.Fail(result, ContractMismatch, "The binding contract assembly manifest is invalid.");
        }

        var requestedFingerprint = Convert.ToHexString(new ReadOnlySpan<byte>((byte*)request->Fingerprint, 32));
        if (!string.Equals(contractFingerprint, requestedFingerprint, StringComparison.Ordinal))
            return ResultWriter.Fail(result, ContractMismatch, "The generated binding fingerprint does not match the requested contract.");

        var assemblyMvids = (byte*)request->AssemblyMvids;
        var loadedAssemblies = state.GetLoadedAssemblies().ToArray();
        for (var index = 0; index < request->AssemblyCount; index++)
        {
            var requestedMvid = new Guid(new ReadOnlySpan<byte>(assemblyMvids + (index * 16), 16));
            if (!Guid.TryParse(contractMvidTexts[index], out var expectedMvid) || expectedMvid != requestedMvid)
            {
                return ResultWriter.Fail(
                    result,
                    ContractMismatch,
                    $"The binding contract expects assembly '{assemblyNames[index]}' with MVID '{expectedMvid}', not '{requestedMvid}'.");
            }

            if (!loadedAssemblies.Any(assembly =>
                string.Equals(assembly.GetName().Name, assemblyNames[index], StringComparison.Ordinal) &&
                assembly.ManifestModule.ModuleVersionId == expectedMvid))
                return ResultWriter.Fail(result, ContractMismatch, $"Assembly '{assemblyNames[index]}' with MVID '{expectedMvid}' was not loaded.");
        }

        return Ok;
    }
}