using NeoWPF.DotNet.Bridge.Runtime;

namespace NeoWPF.DotNet.Bridge.Operations;

internal sealed unsafe class OperationRecord(
    Task<int> task,
    CancellationTokenSource? cancellation,
    nint userData,
    delegate* unmanaged[Cdecl]<nint, NwReference, uint, void> completed)
{
    private readonly object m_gate = new();
    private readonly Task<int> m_task = task;
    private CancellationTokenSource? m_cancellation = cancellation;
    private nint m_userData = userData;
    private delegate* unmanaged[Cdecl]<nint, NwReference, uint, void> m_completed = completed;
    private uint m_state = AbiConstants.OperationPending;
    private int m_result;
    private Exception? m_exception;
    private bool m_resultTaken;

    public NwReference Reference { get; set; }

    public uint State
    {
        get
        {
            lock (m_gate)
                return m_state;
        }
    }

    public void Start()
    {
        _ = m_task.ContinueWith(
            static (task, state) => ((OperationRecord)state!).Complete(task),
            this,
            CancellationToken.None,
            TaskContinuationOptions.None,
            TaskScheduler.Default);
    }

    public int Cancel()
    {
        lock (m_gate)
        {
            if (m_state != AbiConstants.OperationPending)
                return AbiConstants.Ok;
            if (m_cancellation is null)
                return AbiConstants.Unsupported;

            m_cancellation.Cancel();
            return AbiConstants.Ok;
        }
    }

    public int Release()
    {
        lock (m_gate)
        {
            if (m_state == AbiConstants.OperationPending)
                m_cancellation?.Cancel();
            m_userData = 0;
            m_completed = null;
            m_state = AbiConstants.OperationReleased;
            return AbiConstants.Ok;
        }
    }

    public int TakeResult(out int value, out Exception? exception)
    {
        lock (m_gate)
        {
            value = 0;
            exception = null;
            if (m_state == AbiConstants.OperationPending)
                return AbiConstants.InvalidArgument;
            if (m_state == AbiConstants.OperationReleased || m_resultTaken)
                return AbiConstants.InvalidArgument;

            m_resultTaken = true;
            value = m_result;
            exception = m_exception;
            return m_state switch
            {
                AbiConstants.OperationSucceeded => AbiConstants.Ok,
                AbiConstants.OperationFaulted => AbiConstants.ManagedException,
                AbiConstants.OperationCanceled => AbiConstants.Canceled,
                _ => AbiConstants.InternalError,
            };
        }
    }

    private void Complete(Task<int> task)
    {
        delegate* unmanaged[Cdecl]<nint, NwReference, uint, void> callback;
        nint userData;
        uint completedState;
        lock (m_gate)
        {
            if (m_state == AbiConstants.OperationReleased)
                return;

            if (task.IsCanceled)
                m_state = AbiConstants.OperationCanceled;
            else if (task.IsFaulted)
            {
                m_state = AbiConstants.OperationFaulted;
                m_exception = task.Exception?.GetBaseException();
            }
            else
            {
                m_state = AbiConstants.OperationSucceeded;
                m_result = task.GetAwaiter().GetResult();
            }

            m_cancellation?.Dispose();
            m_cancellation = null;
            callback = m_completed;
            userData = m_userData;
            completedState = m_state;
            m_completed = null;
        }

        if (callback != null)
            callback(userData, Reference, completedState);
    }
}