using System.Runtime.InteropServices;
using SecondaryDependency;

namespace PluginAssembly;

public static class PluginService
{
    public static int AddFive(int value)
    {
        return Calculator.AddFive(value);
    }

    public static int AddSeven(int value)
    {
        return AddSevenNative(value);
    }

    [DllImport("NeoWPF.DotNet.TestNativeDependency", EntryPoint = "AddSeven", CallingConvention = CallingConvention.Cdecl)]
    private static extern int AddSevenNative(int value);
}