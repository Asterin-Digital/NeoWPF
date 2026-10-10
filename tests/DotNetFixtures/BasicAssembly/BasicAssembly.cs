namespace BasicAssembly.Domain;

public enum InvoiceState
{
    Draft,
    Paid,
}

public interface IInvoiceCalculator
{
    decimal Calculate(InvoiceRequest request);
}

public sealed class InvoiceRequest
{
    public InvoiceRequest(decimal subtotal)
    {
        Subtotal = subtotal;
    }

    public decimal Subtotal { get; }
}

public readonly struct Money
{
    public Money(decimal amount, string currency)
    {
        Amount = amount;
        Currency = currency;
    }

    public decimal Amount { get; }
    public string Currency { get; }
}

public delegate void InvoiceCallback(InvoiceState state, Money amount);

public sealed class NestedContainer
{
    public sealed class NestedResult
    {
        public string Value { get; init; } = string.Empty;
    }
}

public sealed class InvoiceService
{
    public InvoiceService()
    {
        AccountName = string.Empty;
    }

    public static int AddTen(int template)
    {
        return template + 10;
    }

    public static int Overloaded(int value)
    {
        return value;
    }

    public static int Overloaded(string value)
    {
        return value.Length;
    }

    public static int ThrowManagedException(int value)
    {
        throw new InvalidOperationException($"fixture failure: {value}");
    }

    public static Task<int> AddAsync(int value) => Task.FromResult(value + 5);

    public static Task<int> FailAsync(int value) =>
        Task.FromException<int>(new InvalidOperationException($"async failure: {value}"));

    public static async Task<int> WaitAsync(int value, CancellationToken cancellationToken)
    {
        await Task.Delay(Timeout.Infinite, cancellationToken);
        return value;
    }

    public InvoiceService(string accountName)
    {
        AccountName = accountName;
    }

    public string AccountName { get; }

    public int this[int index]
    {
        get => index;
        set { }
    }

    public static int InstanceCount { get; private set; }

    public decimal Calculate(InvoiceRequest request)
    {
        return request.Subtotal;
    }

    public decimal Calculate(int amount)
    {
        return amount;
    }

    public static InvoiceRequest CreateRequest(decimal subtotal)
    {
        return new InvoiceRequest(subtotal);
    }

    public T Echo<T>(T value)
    {
        return value;
    }

    public decimal Sum(decimal[] values)
    {
        return values.Sum();
    }

    public bool TryGetMoney(decimal amount, out Money money)
    {
        money = new Money(amount, "USD");
        return true;
    }

    public void UpdateMoney(ref Money money, in decimal adjustment)
    {
        money = new Money(money.Amount + adjustment, money.Currency);
    }

    public Task<decimal> CalculateAsync(InvoiceRequest request, CancellationToken cancellationToken)
    {
        return Task.FromResult(request.Subtotal);
    }

    public void SetState(InvoiceState state)
    {
    }

    public event EventHandler<InvoiceState>? StateChanged;

    public void RaiseStateChanged(InvoiceState state)
    {
        StateChanged?.Invoke(this, state);
    }
}

public abstract class AbstractInvoiceService
{
    public abstract decimal Calculate(InvoiceRequest request);
}

public static class InvoiceMath
{
    public static int Double(int value) => value * 2;
}

public ref struct InvoiceSpan
{
    public int Value;
}

public sealed class UtilityService
{
    public static int Double(int value)
    {
        return value * 2;
    }
}
