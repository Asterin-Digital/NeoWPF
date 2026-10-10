using System.Runtime.InteropServices;
using System.Text;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class ResultWriter
{
    public static bool Initialize(NwResult* result)
    {
        if (result is null)
            return false;

        *result = new NwResult
        {
            StructSize = (uint)sizeof(NwResult),
            AbiVersion = AbiConstants.AbiVersion,
            Status = AbiConstants.Ok,
        };
        return true;
    }

    public static int Fail(
        NwResult* result,
        int status,
        string? message = null,
        string? managedTypeName = null,
        string? stackTrace = null)
    {
        if (result is null)
            return status;

        result->Status = status;
        result->Error.StructSize = (uint)sizeof(NwError);
        result->Error.Code = (uint)status;

        NwStringView messageView = default;
        NwStringView typeView = default;
        NwStringView stackView = default;
        try
        {
            messageView = AllocateString(message);
            typeView = AllocateString(managedTypeName);
            stackView = AllocateString(stackTrace);
            result->Error.Message = messageView;
            result->Error.ManagedTypeName = typeView;
            result->Error.StackTrace = stackView;
            return status;
        }
        catch
        {
            Free((void*)messageView.Data);
            Free((void*)typeView.Data);
            Free((void*)stackView.Data);
            result->Error.Message = default;
            result->Error.ManagedTypeName = default;
            result->Error.StackTrace = default;
            return status;
        }
    }

    public static int FailWithoutAllocation(NwResult* result, int status)
    {
        if (result is not null)
        {
            result->Status = status;
            result->Error.StructSize = (uint)sizeof(NwError);
            result->Error.Code = (uint)status;
        }
        return status;
    }

    public static void SetReference(NwResult* result, NwReference reference)
    {
        result->Value.Kind = AbiConstants.ReferenceValueKind;
        result->Value.Payload.Reference = reference;
        result->Status = AbiConstants.Ok;
    }

    public static void SetInt32(NwResult* result, int value)
    {
        result->Value.Kind = AbiConstants.Int32ValueKind;
        result->Value.Payload.Signed = value;
        result->Status = AbiConstants.Ok;
    }

    public static int Release(NwResult* result)
    {
        if (result is null)
            return AbiConstants.InvalidArgument;

        if ((result->Value.Flags & AbiConstants.OwnedValueFlag) != 0)
        {
            switch (result->Value.Kind)
            {
                case AbiConstants.StringValueKind:
                    Free((void*)result->Value.Payload.String.Data);
                    break;
                case AbiConstants.BytesValueKind:
                    Free((void*)result->Value.Payload.Bytes.Data);
                    break;
                case AbiConstants.ArrayValueKind:
                    Free((void*)result->Value.Payload.Array.Data);
                    break;
            }
        }

        Free((void*)result->Error.Message.Data);
        Free((void*)result->Error.ManagedTypeName.Data);
        Free((void*)result->Error.StackTrace.Data);
        *result = default;
        return AbiConstants.Ok;
    }

    public static int ReleaseBuffer(void* buffer)
    {
        Free(buffer);
        return AbiConstants.Ok;
    }

    private static NwStringView AllocateString(string? value)
    {
        if (string.IsNullOrEmpty(value))
            return default;

        var bytes = Encoding.UTF8.GetBytes(value);
        var memory = (byte*)NativeMemory.Alloc((nuint)bytes.Length);
        if (memory is null)
            throw new OutOfMemoryException();

        bytes.CopyTo(new Span<byte>(memory, bytes.Length));
        return new NwStringView
        {
            Data = (nint)memory,
            Length = (ulong)bytes.Length,
        };
    }

    private static void Free(void* memory)
    {
        if (memory is not null)
            NativeMemory.Free(memory);
    }
}