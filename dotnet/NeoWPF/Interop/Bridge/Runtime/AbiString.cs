using System.Text;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal static unsafe class AbiString
{
    public static string Read(NwStringView value)
    {
        if (value.Length > int.MaxValue || (value.Length != 0 && value.Data == 0))
            throw new ArgumentException("The string view is invalid.");
        if (value.Length == 0)
            return string.Empty;

        return Encoding.UTF8.GetString((byte*)value.Data, checked((int)value.Length));
    }
}