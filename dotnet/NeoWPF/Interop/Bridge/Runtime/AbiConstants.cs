namespace NeoWPF.DotNet.Bridge.Runtime;

internal static class AbiConstants
{
    public const uint AbiMajor = 1;
    public const uint AbiMinor = 0;
    public const uint AbiVersion = (AbiMajor << 16) | AbiMinor;

    public const int Ok = 0;
    public const int InvalidArgument = 1;
    public const int AbiMismatch = 2;
    public const int WrongContext = 3;
    public const int StaleReference = 4;
    public const int WrongKind = 5;
    public const int NotFound = 6;
    public const int Ambiguous = 7;
    public const int SignatureMismatch = 8;
    public const int ConversionFailed = 9;
    public const int ManagedException = 10;
    public const int WrongThread = 11;
    public const int RenderThreadForbidden = 12;
    public const int Canceled = 13;
    public const int Unsupported = 14;
    public const int ContractMismatch = 15;
    public const int SessionEnded = 16;
    public const int OutOfMemory = 17;
    public const int InternalError = 18;

    public const uint NoReferenceKind = 0;
    public const uint AssemblyReferenceKind = 1;
    public const uint TypeReferenceKind = 2;
    public const uint MemberReferenceKind = 3;
    public const uint ObjectReferenceKind = 4;
    public const uint DelegateReferenceKind = 5;
    public const uint ExceptionReferenceKind = 6;
    public const uint OperationReferenceKind = 7;

    public const uint NullValueKind = 0;
    public const uint BooleanValueKind = 1;
    public const uint Int8ValueKind = 2;
    public const uint Int16ValueKind = 3;
    public const uint Int32ValueKind = 4;
    public const uint Int64ValueKind = 5;
    public const uint UInt8ValueKind = 6;
    public const uint UInt16ValueKind = 7;
    public const uint UInt32ValueKind = 8;
    public const uint UInt64ValueKind = 9;
    public const uint Float32ValueKind = 10;
    public const uint Float64ValueKind = 11;
    public const uint DecimalValueKind = 12;
    public const uint StringValueKind = 13;
    public const uint BytesValueKind = 14;
    public const uint EnumValueKind = 15;
    public const uint ReferenceValueKind = 16;
    public const uint ArrayValueKind = 17;

    public const uint OwnedValueFlag = 1;
    public const uint ElementKindMask = 0x0000ff00;

    public const uint UnspecifiedPolicy = 0;
    public const uint ManagedPolicy = 1;
    public const uint CallingPolicy = 2;
    public const uint ApplicationPolicy = 3;
    public const uint BackgroundPolicy = 4;

    public const uint OperationPending = 0;
    public const uint OperationSucceeded = 1;
    public const uint OperationFaulted = 2;
    public const uint OperationCanceled = 3;
    public const uint OperationReleased = 4;

    public const uint NamedTypeShape = 0;
    public const uint ArrayTypeShape = 1;
    public const uint NullableTypeShape = 2;
    public const uint GenericTypeShape = 3;

    public const uint StaticMemberFlag = 1;
    public const uint NameOnlyMemberFlag = 2;

    public static bool Validate()
    {
        ReadOnlySpan<int> statuses = [
            Ok, InvalidArgument, AbiMismatch, WrongContext, StaleReference, WrongKind,
            NotFound, Ambiguous, SignatureMismatch, ConversionFailed, ManagedException,
            WrongThread, RenderThreadForbidden, Canceled, Unsupported, ContractMismatch,
            SessionEnded, OutOfMemory, InternalError
        ];
        ReadOnlySpan<int> expectedStatuses = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
        ReadOnlySpan<uint> referenceKinds = [
            NoReferenceKind, AssemblyReferenceKind, TypeReferenceKind, MemberReferenceKind,
            ObjectReferenceKind, DelegateReferenceKind, ExceptionReferenceKind, OperationReferenceKind
        ];
        ReadOnlySpan<uint> expectedReferenceKinds = [0, 1, 2, 3, 4, 5, 6, 7];
        ReadOnlySpan<uint> valueKinds = [
            NullValueKind, BooleanValueKind, Int8ValueKind, Int16ValueKind, Int32ValueKind,
            Int64ValueKind, UInt8ValueKind, UInt16ValueKind, UInt32ValueKind, UInt64ValueKind,
            Float32ValueKind, Float64ValueKind, DecimalValueKind, StringValueKind, BytesValueKind,
            EnumValueKind, ReferenceValueKind, ArrayValueKind
        ];
        ReadOnlySpan<uint> expectedValueKinds = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17];
        ReadOnlySpan<uint> policies = [UnspecifiedPolicy, ManagedPolicy, CallingPolicy, ApplicationPolicy, BackgroundPolicy];
        ReadOnlySpan<uint> expectedPolicies = [0, 1, 2, 3, 4];

        return statuses.SequenceEqual(expectedStatuses) &&
            referenceKinds.SequenceEqual(expectedReferenceKinds) &&
            valueKinds.SequenceEqual(expectedValueKinds) &&
            policies.SequenceEqual(expectedPolicies);
    }
}