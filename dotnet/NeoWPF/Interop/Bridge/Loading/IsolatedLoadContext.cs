using System.Reflection;
using System.Runtime.Loader;

namespace NeoWPF.DotNet.Bridge.Loading;

internal sealed class IsolatedLoadContext : AssemblyLoadContext
{
    private readonly object m_gate = new();
    private readonly Dictionary<string, AssemblyDependencyResolver> m_resolvers = new(StringComparer.OrdinalIgnoreCase);
    private readonly List<(string AssemblyPath, AssemblyDependencyResolver Resolver)> m_resolverOrder = [];

    public IsolatedLoadContext(uint contextId)
        : base($"Isolated-{contextId}", isCollectible: false)
    {
    }

    public Assembly LoadAssembly(string assemblyPath)
    {
        var fullPath = Path.GetFullPath(assemblyPath);
        lock (m_gate)
        {
            if (!m_resolvers.ContainsKey(fullPath))
            {
                var resolver = new AssemblyDependencyResolver(fullPath);
                m_resolvers.Add(fullPath, resolver);
                m_resolverOrder.Add((fullPath, resolver));
            }

            return LoadFromAssemblyPath(fullPath);
        }
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        lock (m_gate)
        {
            foreach (var (_, resolver) in m_resolverOrder)
            {
                var assemblyPath = resolver.ResolveAssemblyToPath(assemblyName);
                if (assemblyPath is not null)
                    return LoadFromAssemblyPath(assemblyPath);
            }
        }
        return null;
    }

    protected override nint LoadUnmanagedDll(string unmanagedDllName)
    {
        lock (m_gate)
        {
            foreach (var (assemblyPath, resolver) in m_resolverOrder)
            {
                var libraryPath = resolver.ResolveUnmanagedDllToPath(unmanagedDllName)
                    ?? Platform.UnmanagedLibraryResolver.ResolveAdjacent(assemblyPath, unmanagedDllName);
                if (libraryPath is not null)
                    return LoadUnmanagedDllFromPath(libraryPath);
            }
        }
        return 0;
    }

}