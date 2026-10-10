using System.Collections.Concurrent;
using NeoWPF.DotNet.Bridge.Runtime;

namespace NeoWPF.DotNet.Bridge.References;

internal sealed class ReferenceTable
{
    private readonly uint m_contextId;
    private readonly ConcurrentDictionary<uint, object> m_values = new();
    private uint m_nextIndex;

    public ReferenceTable(uint contextId)
    {
        m_contextId = contextId;
    }

    public NwReference Add(object value, uint kind)
    {
        var index = Interlocked.Increment(ref m_nextIndex);
        if (index == 0)
            throw new InvalidOperationException("The managed reference table is exhausted.");

        m_values[index] = value;
        return new NwReference
        {
            ContextId = m_contextId,
            Index = index,
            Generation = 1,
            Kind = kind,
        };
    }

    public bool TryGet<T>(NwReference reference, uint kind, out T? value)
        where T : class
    {
        value = null;
        return reference.ContextId == m_contextId &&
            reference.Index != 0 &&
            reference.Generation == 1 &&
            reference.Kind == kind &&
            m_values.TryGetValue(reference.Index, out var storedValue) &&
            (value = storedValue as T) is not null;
    }

    public IEnumerable<object> GetValues() => m_values.Values;

    public void Clear() => m_values.Clear();
}