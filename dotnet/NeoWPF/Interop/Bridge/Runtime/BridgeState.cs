using System.Collections.Concurrent;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;
using NeoWPF.DotNet.Bridge.Loading;
using NeoWPF.DotNet.Bridge.References;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal sealed class BridgeState
{
    private readonly ConcurrentDictionary<string, Assembly> m_assemblies = new(StringComparer.OrdinalIgnoreCase);
    private readonly object m_assemblyGate = new();
    private readonly NwHostCallbacks m_hostCallbacks;

    private BridgeState(uint contextId, NwHostCallbacks hostCallbacks)
    {
        ContextId = contextId;
        m_hostCallbacks = hostCallbacks;
        References = new ReferenceTable(contextId);
    }

    private static BridgeState? s_current;

    public uint ContextId { get; }
    public ReferenceTable References { get; }
    public IsolatedLoadContext? LoadContext { get; private set; }

    public static BridgeState Initialize(uint contextId, NwHostCallbacks hostCallbacks)
    {
        var state = new BridgeState(contextId, hostCallbacks);
        Volatile.Write(ref s_current, state);
        return state;
    }

    public static BridgeState? GetCurrent() => Volatile.Read(ref s_current);

    public Assembly LoadAssembly(string path)
    {
        var fullPath = Path.GetFullPath(path);
        lock (m_assemblyGate)
        {
            if (m_assemblies.TryGetValue(fullPath, out var assembly))
                return assembly;

            LoadContext ??= new IsolatedLoadContext(ContextId);
            assembly = LoadContext.LoadAssembly(fullPath);
            m_assemblies[fullPath] = assembly;
            return assembly;
        }
    }

    public void AddAssemblyReference(Assembly assembly, NwReference reference)
    {
        _ = reference;
        lock (m_assemblyGate)
            m_assemblies.TryAdd(assembly.Location, assembly);
    }

    public IEnumerable<Assembly> GetLoadedAssemblies()
    {
        lock (m_assemblyGate)
            return m_assemblies.Values.ToArray();
    }

    public unsafe void Log(uint level, string category, string message)
    {
        if (m_hostCallbacks.Log == 0)
            return;

        try
        {
            var categoryBytes = Encoding.UTF8.GetBytes(category);
            var messageBytes = Encoding.UTF8.GetBytes(message);
            fixed (byte* categoryData = categoryBytes)
            fixed (byte* messageData = messageBytes)
            {
                var callback = (delegate* unmanaged[Cdecl]<nint, uint, NwStringView, NwStringView, void>)m_hostCallbacks.Log;
                callback(
                    m_hostCallbacks.UserData,
                    level,
                    new NwStringView { Data = (nint)categoryData, Length = (ulong)categoryBytes.Length },
                    new NwStringView { Data = (nint)messageData, Length = (ulong)messageBytes.Length });
            }
        }
        catch
        {
        }
    }

    public void Shutdown()
    {
        References.Clear();
        lock (m_assemblyGate)
        {
            m_assemblies.Clear();
            LoadContext = null;
        }
        Interlocked.CompareExchange(ref s_current, null, this);
    }
}