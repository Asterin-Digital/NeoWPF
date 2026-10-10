using System.Runtime.CompilerServices;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using NeoWPF.DotNet.Bridge.References;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class FunctionTable
{
    private const uint AbiVersion = 1u << 16;
    private const int Ok = 0;
    private const int InvalidArgument = 1;
    private const int WrongContext = 3;
    private const int WrongKind = 5;
    private const int NotFound = 6;
    private const int SignatureMismatch = 8;
    private const int ConversionFailed = 9;
    private const int ManagedException = 10;
    private const int Unsupported = 14;
    private const int ContractMismatch = 15;
    private const int InternalError = 18;
    private const uint AssemblyReferenceKind = 1;
    private const uint TypeReferenceKind = 2;
    private const uint MemberReferenceKind = 3;
    private const uint Int32ValueKind = 4;
    private const uint ReferenceValueKind = 16;
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
        s_api->LoadAssembly = (nint)(delegate* unmanaged[Cdecl]<NwLoadAssemblyRequest*, NwResult*, int>)&LoadAssembly;
        s_api->ResolveType = (nint)(delegate* unmanaged[Cdecl]<NwResolveTypeRequest*, NwResult*, int>)&ResolveType;
        s_api->ResolveMember = (nint)(delegate* unmanaged[Cdecl]<NwResolveMemberRequest*, NwResult*, int>)&ResolveMember;
        s_api->InvokeMember = (nint)(delegate* unmanaged[Cdecl]<NwInvokeRequest*, NwResult*, int>)&InvokeMember;
        s_api->VerifyContract = (nint)(delegate* unmanaged[Cdecl]<NwVerifyContractRequest*, NwResult*, int>)&VerifyContract;
        s_api->ReleaseResult = (nint)(delegate* unmanaged[Cdecl]<NwResult*, int>)&ReleaseResult;
        s_api->ReleaseBuffer = (nint)(delegate* unmanaged[Cdecl]<void*, int>)&ReleaseBuffer;
        s_api->Shutdown = (nint)(delegate* unmanaged[Cdecl]<uint, int>)&Shutdown;
        return s_api;
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int LoadAssembly(NwLoadAssemblyRequest* request, NwResult* result)
    {
        if (!InitializeResult(result) || request is null ||
            request->Header.StructSize < sizeof(NwLoadAssemblyRequest) ||
            request->Header.AbiVersion >> 16 != AbiVersion >> 16)
        {
            return Fail(result, InvalidArgument);
        }

        try
        {
            var state = BridgeState.GetCurrent();
            if (state is null)
                return Fail(result, WrongContext);

            var assembly = state.LoadAssembly(ReadString(request->Path));
            var reference = state.References.Add(assembly, AssemblyReferenceKind);
            SetReferenceResult(result, reference);
            return Ok;
        }
        catch (FileNotFoundException)
        {
            return Fail(result, NotFound);
        }
        catch
        {
            return Fail(result, InternalError);
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ResolveType(NwResolveTypeRequest* request, NwResult* result)
    {
        if (!InitializeResult(result) || request is null ||
            request->Header.StructSize < sizeof(NwResolveTypeRequest) ||
            request->Header.AbiVersion >> 16 != AbiVersion >> 16)
        {
            return Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return Fail(result, WrongContext);

        try
        {
            var fullName = ReadString(request->FullName);
            Type? type;
            if (request->Assembly.Index == 0)
            {
                type = Type.GetType(fullName, throwOnError: false, ignoreCase: false);
            }
            else
            {
                if (!state.References.TryGet<Assembly>(request->Assembly, AssemblyReferenceKind, out var assembly))
                    return Fail(result, request->Assembly.ContextId != state.ContextId ? WrongContext : WrongKind);
                type = assembly!.GetType(fullName, throwOnError: false, ignoreCase: false);
            }

            if (type is null)
                return Fail(result, NotFound);

            SetReferenceResult(result, state.References.Add(type, TypeReferenceKind));
            return Ok;
        }
        catch
        {
            return Fail(result, InternalError);
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ResolveMember(NwResolveMemberRequest* request, NwResult* result)
    {
        if (!InitializeResult(result) || request is null ||
            request->Header.StructSize < sizeof(NwResolveMemberRequest) ||
            request->Header.AbiVersion >> 16 != AbiVersion >> 16 ||
            request->MemberKind != 1 || request->GenericArity != 0 ||
            request->ParameterCount > int.MaxValue ||
            (request->ParameterCount != 0 && request->Parameters == 0))
        {
            return Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return Fail(result, WrongContext);
        if (!state.References.TryGet<Type>(request->Type, TypeReferenceKind, out var type))
            return Fail(result, request->Type.ContextId != state.ContextId ? WrongContext : WrongKind);

        try
        {
            var name = ReadString(request->Name);
            var isStatic = (request->Flags & 1) != 0;
            var bindingFlags = BindingFlags.Public | BindingFlags.DeclaredOnly |
                (isStatic ? BindingFlags.Static : BindingFlags.Instance);
            var candidates = type!.GetMethods(bindingFlags).Where(method => method.Name == name).ToArray();
            var parameters = (NwParameterDescriptor*)request->Parameters;
            var matchingMethods = candidates.Where(method =>
            {
                var methodParameters = method.GetParameters();
                if (methodParameters.Length != request->ParameterCount)
                    return false;

                for (var index = 0; index < methodParameters.Length; index++)
                {
                    if (parameters[index].Passing != 0 ||
                        !state.References.TryGet<Type>(parameters[index].Type, TypeReferenceKind, out var parameterType) ||
                        methodParameters[index].ParameterType != parameterType)
                    {
                        return false;
                    }
                }

                return true;
            }).ToArray();

            if (matchingMethods.Length == 0)
                return Fail(result, candidates.Length == 0 ? NotFound : SignatureMismatch);
            if (matchingMethods.Length != 1)
                return Fail(result, InvalidArgument);

            SetReferenceResult(result, state.References.Add(matchingMethods[0], MemberReferenceKind));
            return Ok;
        }
        catch
        {
            return Fail(result, InternalError);
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int InvokeMember(NwInvokeRequest* request, NwResult* result)
    {
        if (!InitializeResult(result) || request is null ||
            request->Header.StructSize < sizeof(NwInvokeRequest) ||
            request->Header.AbiVersion >> 16 != AbiVersion >> 16 ||
            request->ArgumentCount != 1 || request->Arguments == 0)
        {
            return Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return Fail(result, WrongContext);
        if (!state.References.TryGet<MethodInfo>(request->Member, MemberReferenceKind, out var method))
            return Fail(result, request->Member.ContextId != state.ContextId ? WrongContext : WrongKind);
        if (!method!.IsStatic || request->Target.Index != 0)
            return Fail(result, InvalidArgument);

        var arguments = (NwValue*)request->Arguments;
        if (arguments[0].Kind != Int32ValueKind || method.ReturnType != typeof(int) ||
            method.GetParameters() is not [{ ParameterType: var parameterType }] || parameterType != typeof(int))
        {
            return Fail(result, ConversionFailed);
        }

        try
        {
            var value = checked((int)arguments[0].Payload.Signed);
            var returnedValue = method.Invoke(null, [value]);
            if (returnedValue is not int convertedValue)
                return Fail(result, ConversionFailed);

            result->Value.Kind = Int32ValueKind;
            result->Value.Payload.Signed = convertedValue;
            result->Status = Ok;
            return Ok;
        }
        catch (TargetInvocationException)
        {
            return Fail(result, ManagedException);
        }
        catch (OverflowException)
        {
            return Fail(result, ConversionFailed);
        }
        catch
        {
            return Fail(result, InternalError);
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int VerifyContract(NwVerifyContractRequest* request, NwResult* result)
    {
        if (!InitializeResult(result) || request is null ||
            request->Header.StructSize < sizeof(NwVerifyContractRequest) ||
            request->Header.AbiVersion >> 16 != AbiVersion >> 16 ||
            request->Fingerprint == 0 || request->AssemblyCount == 0 ||
            request->AssemblyMvids == 0)
        {
            return Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return Fail(result, WrongContext);

        try
        {
            var bindingName = ReadString(request->BindingName);
            var bindingAssemblyName = $"{bindingName}.Bindings";
            var bindingAssembly = state.GetLoadedAssemblies()
                .SingleOrDefault(assembly => assembly.GetName().Name == bindingAssemblyName);
            if (bindingAssembly is null)
                return Fail(result, ContractMismatch, $"Binding assembly '{bindingAssemblyName}' was not loaded.");

            var exportedTypes = bindingAssembly.GetExportedTypes();
            var fields = exportedTypes
                .SelectMany(type => type.GetFields(BindingFlags.Public | BindingFlags.Static | BindingFlags.DeclaredOnly))
                .ToArray();
            var contractField = fields.SingleOrDefault(field =>
                field.Name == "ContractFingerprint" && field.FieldType.FullName == typeof(string).FullName);
            if (contractField?.GetValue(null) is not string contractFingerprint)
            {
                var availableFields = string.Join(", ", fields.Select(field =>
                    $"{field.DeclaringType?.FullName}.{field.Name}:{field.FieldType.FullName}"));
                return Fail(
                    result,
                    ContractMismatch,
                    $"The generated binding assembly does not expose its contract fingerprint. Fields: {availableFields}");
            }

            var requestedFingerprint = Convert.ToHexString(new ReadOnlySpan<byte>((byte*)request->Fingerprint, 32));
            if (!string.Equals(contractFingerprint, requestedFingerprint, StringComparison.Ordinal))
                return Fail(result, ContractMismatch, "The generated binding fingerprint does not match the requested contract.");

            var assemblyMvids = (byte*)request->AssemblyMvids;
            for (var index = 0; index < request->AssemblyCount; index++)
            {
                var requestedMvid = new Guid(new ReadOnlySpan<byte>(assemblyMvids + (index * 16), 16));
                if (!state.GetLoadedAssemblies().Any(assembly => assembly.ManifestModule.ModuleVersionId == requestedMvid))
                {
                    return Fail(
                        result,
                        ContractMismatch,
                        $"The loaded assemblies do not contain the contract MVID '{requestedMvid}'.");
                }
            }

            return Ok;
        }
        catch
        {
            return Fail(result, InternalError);
        }
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseResult(NwResult* result)
    {
        if (result is null)
            return InvalidArgument;

        if (result->Error.Message.Data != 0)
            NativeMemory.Free((void*)result->Error.Message.Data);
        *result = default;
        return Ok;
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int ReleaseBuffer(void* buffer) => Ok;

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int Shutdown(uint drainTimeoutMilliseconds)
    {
        _ = drainTimeoutMilliseconds;
        BridgeState.GetCurrent()?.Shutdown();
        return Ok;
    }

    private static bool InitializeResult(NwResult* result)
    {
        if (result is null)
            return false;

        *result = new NwResult
        {
            StructSize = (uint)sizeof(NwResult),
            AbiVersion = AbiVersion,
            Status = Ok,
        };
        return true;
    }

    private static int Fail(NwResult* result, int status, string? message = null)
    {
        if (result is not null)
        {
            result->Status = status;
            result->Error.StructSize = (uint)sizeof(NwError);
            result->Error.Code = (uint)status;
            if (message is not null)
            {
                var bytes = Encoding.UTF8.GetBytes(message);
                var messageBuffer = (byte*)NativeMemory.Alloc((nuint)bytes.Length);
                if (messageBuffer is not null)
                {
                    bytes.CopyTo(new Span<byte>(messageBuffer, bytes.Length));
                    result->Error.Message = new NwStringView
                    {
                        Data = (nint)messageBuffer,
                        Length = (ulong)bytes.Length,
                    };
                }
            }
        }
        return status;
    }

    private static void SetReferenceResult(NwResult* result, NwReference reference)
    {
        result->Value.Kind = ReferenceValueKind;
        result->Value.Flags = 1;
        result->Value.Payload.Reference = reference;
        result->Status = Ok;
    }

    private static string ReadString(NwStringView value)
    {
        if (value.Length > int.MaxValue || (value.Length != 0 && value.Data == 0))
            throw new ArgumentException("The string view is invalid.");
        if (value.Length == 0)
            return string.Empty;

        return Encoding.UTF8.GetString((byte*)value.Data, checked((int)value.Length));
    }
}