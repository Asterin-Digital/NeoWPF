using System.Reflection;
using System.Runtime.Loader;

namespace NeoWPF.DotNet.Bridge.Loading;

internal sealed class IsolatedLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver m_resolver;

    public IsolatedLoadContext(string mainAssemblyPath, uint contextId)
        : base($"Isolated-{contextId}", isCollectible: false)
    {
        m_resolver = new AssemblyDependencyResolver(mainAssemblyPath);
    }

    protected override Assembly? Load(AssemblyName assemblyName)
    {
        var assemblyPath = m_resolver.ResolveAssemblyToPath(assemblyName);
        return assemblyPath is null ? null : LoadFromAssemblyPath(assemblyPath);
    }
}