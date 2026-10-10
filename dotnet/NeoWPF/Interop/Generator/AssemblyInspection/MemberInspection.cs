using System.Reflection;

namespace NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

public sealed record InspectedParameter(
    string Name,
    InspectedTypeRef Type,
    bool IsByReference,
    bool IsOut,
    bool IsIn)
{
    public string TypeName => Type.Kind == InspectedTypeRefKind.ByReference && Type.Element is not null
        ? Type.Element.DisplayName
        : Type.DisplayName;
}

public sealed record InspectedMethod(
    string Name,
    InspectedTypeRef ReturnType,
    IReadOnlyList<InspectedParameter> Parameters,
    bool IsStatic,
    bool IsConstructor,
    int GenericArity)
{
    public string ReturnTypeName => ReturnType.DisplayName;
}

public sealed record InspectedProperty(
    string Name,
    InspectedTypeRef PropertyType,
    bool CanRead,
    bool CanWrite,
    bool IsStatic,
    IReadOnlyList<InspectedParameter> IndexParameters)
{
    public string PropertyTypeName => PropertyType.DisplayName;
}

public sealed record InspectedEvent(
    string Name,
    InspectedTypeRef HandlerType,
    bool IsStatic)
{
    public string HandlerTypeName => HandlerType.DisplayName;
}

public sealed record InspectedType(
    string FullName,
    string AssemblyName,
    bool IsClass,
    bool IsStruct,
    bool IsEnum,
    bool IsDelegate,
    bool IsInterface,
    bool IsAbstract,
    bool IsStatic,
    bool IsByRefLike,
    bool IsVisible,
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
                .Where(method => !method.IsSpecialName)
                .Select(method => InspectMethod(method, method.GetParameters(), isConstructor: false)))
            .ToArray();

        var properties = type.GetProperties(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
            .Select(property => new InspectedProperty(
                property.Name,
                InspectedTypeRef.FromReflection(property.PropertyType),
                property.GetMethod is not null,
                property.SetMethod is not null,
                (property.GetMethod ?? property.SetMethod)?.IsStatic == true,
                property.GetIndexParameters().Select(parameter => new InspectedParameter(
                    parameter.Name ?? string.Empty,
                    InspectedTypeRef.FromReflection(parameter.ParameterType),
                    parameter.ParameterType.IsByRef,
                    parameter.IsOut,
                    parameter.IsIn)).ToArray()))
            .ToArray();

        var events = type.GetEvents(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
            .Select(@event => new InspectedEvent(
                @event.Name,
                InspectedTypeRef.FromReflection(@event.EventHandlerType ?? typeof(Delegate)),
                (@event.AddMethod ?? @event.RemoveMethod)?.IsStatic == true))
            .ToArray();

        return new InspectedType(
            type.FullName ?? type.Name,
            type.Assembly.GetName().Name ?? string.Empty,
            type.IsClass,
            type.IsValueType && !type.IsEnum,
            type.IsEnum,
            delegateType.IsAssignableFrom(type),
            type.IsInterface,
            type.IsAbstract,
            type.IsAbstract && type.IsSealed,
            type.IsByRefLike,
            type.IsVisible,
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
            InspectedTypeRef.FromReflection(isConstructor ? method.DeclaringType! : ((MethodInfo)method).ReturnType),
            parameters.Select(parameter => new InspectedParameter(
                parameter.Name ?? string.Empty,
                InspectedTypeRef.FromReflection(parameter.ParameterType),
                parameter.ParameterType.IsByRef,
                parameter.IsOut,
                parameter.IsIn)).ToArray(),
            !isConstructor && method.IsStatic,
            isConstructor,
            !isConstructor && ((MethodInfo)method).IsGenericMethodDefinition
                ? ((MethodInfo)method).GetGenericArguments().Length
                : 0);
}
