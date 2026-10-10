namespace NeoWPF.DotNet.Interop.Generator.Diagnostics;

public static class BindingDiagnosticIds
{
    public const string UnexpectedToken = "NWB1010";
    public const string ExpectedIdentifier = "NWB1011";
    public const string EscapedIdentifierRequired = "NWB1012";
    public const string NewlineInString = "NWB1020";
    public const string UnterminatedEscape = "NWB1021";
    public const string UnsupportedEscape = "NWB1022";
    public const string UnterminatedString = "NWB1023";
    public const string InvalidUnicodeEscape = "NWB1024";
    public const string CSharpKeywordAlias = "NWB1030";
    public const string InvalidPropertyDeclaration = "NWB1031";
    public const string InvalidTuple = "NWB1032";
    public const string InvalidMemberOption = "NWB1033";
    public const string UnterminatedBlockComment = "NWB1040";
    public const string DuplicateType = "NWB2001";
    public const string DuplicateMethod = "NWB2002";
    public const string DuplicateProperty = "NWB2003";
    public const string DuplicateEvent = "NWB2004";
    public const string MissingAssembly = "NWB2010";
    public const string TypeNotFound = "NWB2011";
    public const string MethodNotFound = "NWB2012";
    public const string AmbiguousMethod = "NWB2013";
    public const string SignatureMismatch = "NWB2014";
    public const string ParameterPassingModeCount = "NWB2015";
    public const string AmbiguousType = "NWB2016";
    public const string DuplicateMemberOption = "NWB2020";
    public const string DuplicateBindingOrTypeOption = "NWB2021";
    public const string UnsupportedType = "NWB3010";
    public const string UnresolvedTypeShape = "NWB3011";
    public const string UnknownPrimitiveType = "NWB3012";
    public const string InvalidArrayShape = "NWB3013";
    public const string InvalidWrappedType = "NWB3014";
    public const string InvalidTupleShape = "NWB3015";
    public const string InvalidTaskShape = "NWB3016";
    public const string GenericArityMismatch = "NWB3017";
    public const string ReturnTypeMismatch = "NWB3018";
    public const string ParameterTypeMismatch = "NWB3019";
    public const string UnsupportedByReference = "NWB4020";
    public const string UnknownExecutionPolicy = "NWB4021";
    public const string UnknownCallbackPolicy = "NWB4022";
    public const string UnknownOwnership = "NWB4023";
    public const string UnknownPassingMode = "NWB4024";
    public const string AsyncReturnMismatch = "NWB4025";
    public const string MissingAsyncMarker = "NWB4026";
    public const string InvalidEventParameter = "NWB4027";
    public const string BindingCountLimit = "NWB4030";
    public const string TypeCountLimit = "NWB4031";
    public const string MethodCountLimit = "NWB4032";
    public const string UnsupportedExecutionPolicy = "NWB4033";
    public const string UnsupportedFeature = "NWB4034";
    public const string InvalidEmittedIdentifier = "NWB5001";
    public const string CppKeyword = "NWB5002";
    public const string EmittedNameCollision = "NWB5003";

    public static IReadOnlyList<string> All { get; } =
    [
        UnexpectedToken, ExpectedIdentifier, EscapedIdentifierRequired, NewlineInString,
        UnterminatedEscape, UnsupportedEscape, UnterminatedString, InvalidUnicodeEscape,
        CSharpKeywordAlias, InvalidPropertyDeclaration, InvalidTuple, InvalidMemberOption,
        UnterminatedBlockComment, DuplicateType, DuplicateMethod, DuplicateProperty, DuplicateEvent,
        MissingAssembly, TypeNotFound, MethodNotFound, AmbiguousMethod,
        SignatureMismatch, ParameterPassingModeCount, AmbiguousType, DuplicateMemberOption, DuplicateBindingOrTypeOption,
        UnsupportedType, UnresolvedTypeShape, UnknownPrimitiveType, InvalidArrayShape, InvalidWrappedType,
        InvalidTupleShape, InvalidTaskShape, GenericArityMismatch, ReturnTypeMismatch, ParameterTypeMismatch,
        UnsupportedByReference, UnknownExecutionPolicy, UnknownCallbackPolicy, UnknownOwnership,
        UnknownPassingMode, AsyncReturnMismatch, MissingAsyncMarker, InvalidEventParameter,
        BindingCountLimit, TypeCountLimit, MethodCountLimit, UnsupportedExecutionPolicy, UnsupportedFeature,
        InvalidEmittedIdentifier, CppKeyword, EmittedNameCollision,
    ];
}
