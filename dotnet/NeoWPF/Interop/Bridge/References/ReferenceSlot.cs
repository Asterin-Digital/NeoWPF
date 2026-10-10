namespace NeoWPF.DotNet.Bridge.References;

internal sealed class ReferenceSlot(
    object value,
    uint kind,
    bool isPinned)
{
    public object Value { get; } = value;
    public uint Kind { get; } = kind;
    public uint Generation { get; } = 1;
    public uint ReferenceCount { get; set; } = 1;
    public uint LeaseCount { get; set; }
    public bool IsPinned { get; } = isPinned;
    public bool PendingFree { get; set; }
}