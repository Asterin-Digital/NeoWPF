using NeoWPF.DotNet.Bridge.Runtime;
using static NeoWPF.DotNet.Bridge.Runtime.AbiConstants;

namespace NeoWPF.DotNet.Bridge.References;

internal sealed class ReferenceTable
{
    private readonly uint m_contextId;
    private readonly Dictionary<uint, ReferenceSlot> m_values = new();
    private readonly Dictionary<(uint Kind, object Value), NwReference> m_pinnedReferences = new();
    private readonly object m_gate = new();
    private uint m_nextIndex;

    public ReferenceTable(uint contextId)
    {
        m_contextId = contextId;
    }

    public NwReference Add(object value, uint kind)
    {
        lock (m_gate)
        {
            var isPinned = kind is AssemblyReferenceKind or TypeReferenceKind or MemberReferenceKind;
            var key = (kind, value);
            if (isPinned && m_pinnedReferences.TryGetValue(key, out var existing))
                return existing;

            var index = checked(++m_nextIndex);
            if (index == 0)
                throw new InvalidOperationException("The managed reference table is exhausted.");

            var reference = new NwReference
            {
                ContextId = m_contextId,
                Index = index,
                Generation = 1,
                Kind = kind,
            };
            m_values.Add(index, new ReferenceSlot(value, kind, isPinned));
            if (isPinned)
                m_pinnedReferences.Add(key, reference);
            return reference;
        }
    }

    public int GetStatus(NwReference reference, uint kind)
    {
        lock (m_gate)
            return GetStatusLocked(reference, kind);
    }

    public bool TryGet<T>(NwReference reference, uint kind, out T? value)
        where T : class
    {
        lock (m_gate)
        {
            value = null;
            return GetStatusLocked(reference, kind) == Ok &&
                m_values.TryGetValue(reference.Index, out var slot) &&
                (value = slot.Value as T) is not null;
        }
    }

    public int TryAcquire<T>(NwReference reference, uint kind, out T? value, out ReferenceLease? lease)
        where T : class
    {
        lock (m_gate)
        {
            value = null;
            lease = null;
            var status = GetStatusLocked(reference, kind);
            if (status != Ok)
                return status;

            var slot = m_values[reference.Index];
            if (slot.Value is not T typedValue)
                return WrongKind;

            var newLease = new ReferenceLease(this, reference);
            slot.LeaseCount = checked(slot.LeaseCount + 1);
            value = typedValue;
            lease = newLease;
            return Ok;
        }
    }

    public int Retain(NwReference reference)
    {
        lock (m_gate)
        {
            var status = GetStatusLocked(reference, reference.Kind);
            if (status != Ok)
                return status;

            var slot = m_values[reference.Index];
            if (slot.IsPinned)
                return Ok;
            if (slot.ReferenceCount == uint.MaxValue)
                return OutOfMemory;
            slot.ReferenceCount++;
            return Ok;
        }
    }

    public int Release(NwReference reference) => Release(reference, out _);

    public int Release(NwReference reference, out bool fullyReleased)
    {
        fullyReleased = false;
        lock (m_gate)
        {
            var status = GetStatusLocked(reference, reference.Kind);
            if (status != Ok)
                return status;

            var slot = m_values[reference.Index];
            if (slot.IsPinned)
                return Ok;
            if (slot.ReferenceCount == 0)
                return StaleReference;

            slot.ReferenceCount--;
            if (slot.ReferenceCount == 0)
            {
                if (slot.LeaseCount == 0)
                {
                    m_values.Remove(reference.Index);
                    fullyReleased = true;
                }
                else
                    slot.PendingFree = true;
            }
            return Ok;
        }
    }

    public IEnumerable<object> GetValues()
    {
        lock (m_gate)
            return m_values.Values.Select(slot => slot.Value).ToArray();
    }

    internal void ReleaseLease(NwReference reference)
    {
        lock (m_gate)
        {
            if (reference.ContextId != m_contextId ||
                !m_values.TryGetValue(reference.Index, out var slot) ||
                slot.Generation != reference.Generation ||
                slot.Kind != reference.Kind || slot.LeaseCount == 0)
            {
                return;
            }

            slot.LeaseCount--;
            if (slot.PendingFree && slot.LeaseCount == 0)
                m_values.Remove(reference.Index);
        }
    }

    private int GetStatusLocked(NwReference reference, uint kind)
    {
        if (reference.ContextId != m_contextId)
            return WrongContext;
        if (reference.Kind != kind)
            return WrongKind;
        if (reference.Index == 0 ||
            !m_values.TryGetValue(reference.Index, out var slot) ||
            slot.Generation != reference.Generation ||
            slot.Kind != reference.Kind)
        {
            return StaleReference;
        }
        return Ok;
    }

    public void Clear()
    {
        lock (m_gate)
        {
            m_values.Clear();
            m_pinnedReferences.Clear();
        }
    }
}