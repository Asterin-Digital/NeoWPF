using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.Types;

internal static unsafe class MemberResolution
{
    public static int ResolveType(NwResolveTypeRequest* request, NwResult* result)
    {
        if (request is null)
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }
        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwResolveTypeRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);

        if (request->Shape != NamedTypeShape || request->TypeArgumentCount != 0 ||
            request->TypeArguments != 0 || request->ArrayRank != 0 ||
            !RequestValidation.IsNull(request->ElementType))
        {
            return ResultWriter.Fail(result, Unsupported);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);

        var fullName = AbiString.Read(request->FullName);
        Type? type;
        if (RequestValidation.IsNull(request->Assembly))
        {
            type = Type.GetType(fullName, throwOnError: false, ignoreCase: false) ??
                AssemblyLoadContext.Default.Assemblies
                    .Select(assembly => assembly.GetType(fullName, throwOnError: false, ignoreCase: false))
                    .FirstOrDefault(candidate => candidate is not null);
        }
        else
        {
            var status = state.References.GetStatus(request->Assembly, AssemblyReferenceKind);
            if (status != Ok)
                return ResultWriter.Fail(result, status);
            if (!state.References.TryGet<Assembly>(request->Assembly, AssemblyReferenceKind, out var assembly))
                return ResultWriter.Fail(result, WrongKind);
            type = assembly!.GetType(fullName, throwOnError: false, ignoreCase: false);
        }

        if (type is null)
            return ResultWriter.Fail(result, NotFound);

        ResultWriter.SetReference(result, state.References.Add(type, TypeReferenceKind));
        return Ok;
    }

    public static int ResolveMember(NwResolveMemberRequest* request, NwResult* result)
    {
        if (request is null)
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }
        var headerStatus = RequestValidation.ValidateHeader(&request->Header, (uint)sizeof(NwResolveMemberRequest));
        if (headerStatus != Ok)
            return ResultWriter.Fail(result, headerStatus);

        if (request->MemberKind is not (0 or 1) || request->GenericArity != 0 || request->GenericArguments != 0)
            return ResultWriter.Fail(result, Unsupported);
        if ((request->Flags & ~(StaticMemberFlag | NameOnlyMemberFlag)) != 0 ||
            request->ParameterCount > int.MaxValue ||
            (request->ParameterCount != 0 && request->Parameters == 0))
        {
            return ResultWriter.Fail(result, InvalidArgument);
        }

        var state = BridgeState.GetCurrent();
        if (state is null)
            return ResultWriter.Fail(result, SessionEnded);
        var typeStatus = state.References.GetStatus(request->Type, TypeReferenceKind);
        if (typeStatus != Ok)
            return ResultWriter.Fail(result, typeStatus);
        if (!state.References.TryGet<Type>(request->Type, TypeReferenceKind, out var type))
            return ResultWriter.Fail(result, WrongKind);

        var nameOnly = (request->Flags & NameOnlyMemberFlag) != 0;
        if (nameOnly && (request->ParameterCount != 0 || request->Parameters != 0))
            return ResultWriter.Fail(result, InvalidArgument);

        var parameterTypes = new Type[request->ParameterCount];
        var descriptors = (NwParameterDescriptor*)request->Parameters;
        for (var index = 0; index < parameterTypes.Length; index++)
        {
            var descriptor = descriptors[index];
            var parameterStatus = state.References.GetStatus(descriptor.Type, TypeReferenceKind);
            if (parameterStatus != Ok)
                return ResultWriter.Fail(result, parameterStatus);
            if (!state.References.TryGet<Type>(descriptor.Type, TypeReferenceKind, out var parameterType))
                return ResultWriter.Fail(result, WrongKind);
            if (descriptor.Reserved != 0)
                return ResultWriter.Fail(result, InvalidArgument);
            if (descriptor.Passing != 0)
                return ResultWriter.Fail(result, Unsupported);
            parameterTypes[index] = parameterType!;
        }

        var name = AbiString.Read(request->Name);
        var isStatic = (request->Flags & StaticMemberFlag) != 0;
        if (request->MemberKind == 0)
        {
            if (isStatic || nameOnly || name.Length != 0)
                return ResultWriter.Fail(result, InvalidArgument);

            var constructors = type!.GetConstructors(BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly);
            var matchingConstructors = constructors
                .Where(constructor => HasExactParameters(constructor, parameterTypes))
                .ToArray();
            if (matchingConstructors.Length == 0)
            {
                var candidates = string.Join("; ", constructors
                    .Select(FormatSignature)
                    .OrderBy(signature => signature, StringComparer.Ordinal));
                return ResultWriter.Fail(
                    result,
                    constructors.Length == 0 ? NotFound : SignatureMismatch,
                    $"No matching constructor on '{type.FullName}'. Candidates: {candidates}");
            }
            if (matchingConstructors.Length > 1)
            {
                var signatures = string.Join("; ", matchingConstructors
                    .Select(FormatSignature)
                    .OrderBy(signature => signature, StringComparer.Ordinal));
                return ResultWriter.Fail(result, Ambiguous, $"The constructor on '{type.FullName}' is ambiguous: {signatures}");
            }

            ResultWriter.SetReference(result, state.References.Add(matchingConstructors[0], MemberReferenceKind));
            return Ok;
        }

        var flags = BindingFlags.Public | BindingFlags.DeclaredOnly |
            (isStatic ? BindingFlags.Static : BindingFlags.Instance);
        var namedMethods = type!.GetMethods(flags)
            .Where(method => method.Name == name && !method.IsSpecialName)
            .ToArray();
        if (namedMethods.Any(method => method.IsGenericMethodDefinition))
            return ResultWriter.Fail(result, Unsupported);

        var matchingMethods = nameOnly
            ? namedMethods
            : namedMethods.Where(method => HasExactParameters(method, parameterTypes)).ToArray();
        if (matchingMethods.Length == 0)
        {
            var candidates = string.Join("; ", namedMethods
                .Select(FormatSignature)
                .OrderBy(signature => signature, StringComparer.Ordinal));
            return ResultWriter.Fail(
                result,
                namedMethods.Length == 0 ? NotFound : SignatureMismatch,
                $"No matching member '{name}' on '{type!.FullName}'. Candidates: {candidates}");
        }
        if (matchingMethods.Length > 1)
        {
            var signatures = string.Join("; ", matchingMethods
                .Select(FormatSignature)
                .OrderBy(signature => signature, StringComparer.Ordinal));
            return ResultWriter.Fail(result, Ambiguous, $"The member '{name}' is ambiguous: {signatures}");
        }

        ResultWriter.SetReference(result, state.References.Add(matchingMethods[0], MemberReferenceKind));
        return Ok;
    }

    private static bool HasExactParameters(MethodInfo method, IReadOnlyList<Type> parameterTypes)
        => HasExactParameters((MethodBase)method, parameterTypes);

    private static bool HasExactParameters(MethodBase method, IReadOnlyList<Type> parameterTypes)
    {
        var parameters = method.GetParameters();
        if (parameters.Length != parameterTypes.Count)
            return false;

        for (var index = 0; index < parameters.Length; index++)
        {
            if (parameters[index].ParameterType != parameterTypes[index])
                return false;
        }

        return true;
    }

    private static string FormatSignature(MethodInfo method)
    {
        var parameters = string.Join(", ", method.GetParameters().Select(parameter =>
            parameter.ParameterType.FullName ?? parameter.ParameterType.Name));
        return $"{method.ReturnType.FullName} {method.Name}({parameters})";
    }

    private static string FormatSignature(ConstructorInfo constructor)
    {
        var parameters = string.Join(", ", constructor.GetParameters().Select(parameter =>
            parameter.ParameterType.FullName ?? parameter.ParameterType.Name));
        return $"{constructor.DeclaringType?.FullName}({parameters})";
    }
}