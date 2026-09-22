using System.Reflection;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public sealed record InspectedParameter(
    string Name,
    string TypeName,
    bool IsByReference,
    bool IsOut,
    bool IsIn);

public sealed record InspectedMethod(
    string Name,
    string ReturnTypeName,
    IReadOnlyList<InspectedParameter> Parameters,
    bool IsStatic,
    bool IsConstructor,
    int GenericArity);

public sealed record InspectedProperty(
    string Name,
    string PropertyTypeName,
    bool CanRead,
    bool CanWrite,
    bool IsStatic);

public sealed record InspectedEvent(
    string Name,
    string HandlerTypeName,
    bool IsStatic);

public sealed record InspectedType(
    string FullName,
    bool IsClass,
    bool IsStruct,
    bool IsEnum,
    bool IsDelegate,
    int GenericArity,
    IReadOnlyList<InspectedMethod> Methods,
    IReadOnlyList<InspectedProperty> Properties,
    IReadOnlyList<InspectedEvent> Events);

internal static class MemberInspection
{
    public static InspectedType InspectType(Type type, Type delegateType)
    {
        var methods = type.GetConstructors(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static)
            .Select(constructor => InspectMethod(constructor, constructor.GetParameters(), isConstructor: true))
            .Concat(type.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
                .Select(method => InspectMethod(method, method.GetParameters(), isConstructor: false)))
            .ToArray();

        var properties = type.GetProperties(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
            .Select(property => new InspectedProperty(
                property.Name,
                property.PropertyType.FullName ?? property.PropertyType.Name,
                property.GetMethod is not null,
                property.SetMethod is not null,
                (property.GetMethod ?? property.SetMethod)?.IsStatic == true))
            .ToArray();

        var events = type.GetEvents(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
            .Select(@event => new InspectedEvent(
                @event.Name,
                @event.EventHandlerType?.FullName ?? @event.EventHandlerType?.Name ?? string.Empty,
                (@event.AddMethod ?? @event.RemoveMethod)?.IsStatic == true))
            .ToArray();

        return new InspectedType(
            type.FullName ?? type.Name,
            type.IsClass,
            type.IsValueType && !type.IsEnum,
            type.IsEnum,
            delegateType.IsAssignableFrom(type),
            type.IsGenericTypeDefinition ? type.GetGenericArguments().Length : 0,
            methods,
            properties,
            events);
    }

    private static InspectedMethod InspectMethod(
        MethodBase method,
        IReadOnlyList<ParameterInfo> parameters,
        bool isConstructor) =>
        new(
            isConstructor ? ".ctor" : method.Name,
            isConstructor ? method.DeclaringType?.FullName ?? string.Empty : ((MethodInfo)method).ReturnType.FullName ?? ((MethodInfo)method).ReturnType.Name,
            parameters.Select(parameter => new InspectedParameter(
                parameter.Name ?? string.Empty,
                parameter.ParameterType.IsByRef
                    ? parameter.ParameterType.GetElementType()?.FullName ?? parameter.ParameterType.GetElementType()?.Name ?? string.Empty
                    : parameter.ParameterType.FullName ?? parameter.ParameterType.Name,
                parameter.ParameterType.IsByRef,
                parameter.IsOut,
                parameter.IsIn)).ToArray(),
            !isConstructor && method.IsStatic,
            isConstructor,
            !isConstructor && ((MethodInfo)method).IsGenericMethodDefinition
                ? ((MethodInfo)method).GetGenericArguments().Length
                : 0);
}
