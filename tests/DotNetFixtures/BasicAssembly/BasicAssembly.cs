namespace BasicAssembly.Domain;

public enum InvoiceState
{
    Draft,
    Paid,
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
    public static int AddTen(int value)
    {
        return value + 10;
    }

    public InvoiceService(string accountName)
    {
        AccountName = accountName;
    }

    public string AccountName { get; }

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
