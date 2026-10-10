using NeoWPF.DotNet.Bridge.Runtime;

namespace NeoWPF.DotNet.Bridge.References;

internal sealed class ReferenceLease : IDisposable
{
    private ReferenceTable? m_table;
    private readonly NwReference m_reference;

    public ReferenceLease(ReferenceTable table, NwReference reference)
    {
        m_table = table;
        m_reference = reference;
    }

    public void Dispose()
    {
        Interlocked.Exchange(ref m_table, null)?.ReleaseLease(m_reference);
    }
}