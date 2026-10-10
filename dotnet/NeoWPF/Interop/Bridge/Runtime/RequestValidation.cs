namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class RequestValidation
{
    public static int ValidateHeader(NwRequestHeader* header, uint minimumSize)
    {
        if (header is null)
            return AbiConstants.InvalidArgument;
        if (header->AbiVersion >> 16 != AbiConstants.AbiMajor)
            return AbiConstants.AbiMismatch;
        if (header->StructSize < minimumSize)
            return AbiConstants.InvalidArgument;
        return AbiConstants.Ok;
    }

    public static bool IsNull(NwReference reference) =>
        reference.ContextId == 0 &&
        reference.Index == 0 &&
        reference.Generation == 0 &&
        reference.Kind == AbiConstants.NoReferenceKind;
}