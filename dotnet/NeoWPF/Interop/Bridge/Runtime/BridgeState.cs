using System.Collections.Concurrent;
using System.Reflection;
using NeoWPF.DotNet.Bridge.Loading;
using NeoWPF.DotNet.Bridge.References;

namespace NeoWPF.DotNet.Bridge.Runtime;

internal sealed class BridgeState
{
    private readonly ConcurrentDictionary<string, Assembly> m_assemblies = new(StringComparer.OrdinalIgnoreCase);

    private BridgeState(uint contextId)
    {
        ContextId = contextId;
        References = new ReferenceTable(contextId);
    }

    private static BridgeState? s_current;

    public uint ContextId { get; }
    public ReferenceTable References { get; }
    public IsolatedLoadContext? LoadContext { get; private set; }

    public static BridgeState Initialize(uint contextId)
    {
        var state = new BridgeState(contextId);
        Volatile.Write(ref s_current, state);
        return state;
    }

    public static BridgeState? GetCurrent() => Volatile.Read(ref s_current);

    public Assembly LoadAssembly(string path)
    {
        var fullPath = Path.GetFullPath(path);
        return m_assemblies.GetOrAdd(fullPath, resolvedPath =>
        {
            LoadContext ??= new IsolatedLoadContext(resolvedPath, ContextId);
            return LoadContext.LoadFromAssemblyPath(resolvedPath);
        });
    }

    public void AddAssemblyReference(Assembly assembly, NwReference reference)
    {
        _ = reference;
        m_assemblies.TryAdd(assembly.Location, assembly);
    }

    public IEnumerable<Assembly> GetLoadedAssemblies() => m_assemblies.Values;

    public void Shutdown()
    {
        References.Clear();
        m_assemblies.Clear();
        LoadContext = null;
        Interlocked.CompareExchange(ref s_current, null, this);
    }
}