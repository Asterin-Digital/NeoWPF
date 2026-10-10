using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;

namespace NeoWPF.DotNet.Interop.Generator.Resolution;

internal static class ContractFingerprint
{
    private const string ContractVersion = "1";

    public static (string Contract, string Fingerprint) Create(ResolvedBinding resolvedBinding)
    {
        var builder = new StringBuilder();
        Append(builder, "binding", resolvedBinding.Binding.Name);
        Append(builder, "native-namespace", resolvedBinding.Binding.NativeName ?? resolvedBinding.Binding.Name);
        AppendIdentity(builder, "assembly", resolvedBinding.PrimaryAssembly);
        foreach (var dependency in GetAssemblyIdentities(resolvedBinding).Skip(1))
        {
            AppendIdentity(builder, "dependency", dependency);
        }

        Append(builder, "binding-version", resolvedBinding.Binding.Version ?? string.Empty);
        foreach (var resolvedType in resolvedBinding.Types)
        {
            foreach (var resolvedMethod in resolvedType.Methods)
            {
                Append(builder, "type", resolvedType.Type.Name);
                Append(builder, "native-type", resolvedType.Declaration.NativeName);
                Append(builder, "method", resolvedMethod.Method.Name);
                Append(builder, "method-static", resolvedMethod.Method.IsStatic ? "true" : "false");
                Append(builder, "native-method", resolvedMethod.Declaration.NativeName);
                Append(builder, "return", resolvedMethod.Method.ReturnTypeName);
                Append(builder, "execution", resolvedMethod.Declaration.Execution.ToString());
                foreach (var parameter in resolvedMethod.Method.Parameters)
                {
                    Append(
                        builder,
                        "parameter",
                        $"{parameter.TypeName}:{parameter.IsByReference}:{parameter.IsOut}:{parameter.IsIn}");
                }
            }
        }

        Append(builder, "contract-version", ContractVersion);
        Append(builder, "generator-version", typeof(ContractFingerprint).Assembly.GetName().Version?.ToString() ?? "0.0.0.0");
        var contract = builder.ToString();
        var fingerprint = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(contract)));
        return (contract, fingerprint);
    }

    public static IReadOnlyList<AssemblyIdentity> GetAssemblyIdentities(ResolvedBinding resolvedBinding) =>
        new[] { resolvedBinding.PrimaryAssembly }
            .Concat(resolvedBinding.Dependencies
                .OrderBy(identity => identity.Name, StringComparer.Ordinal)
                .ThenBy(identity => identity.Version?.ToString(), StringComparer.Ordinal)
                .ThenBy(identity => identity.Culture, StringComparer.Ordinal)
                .ThenBy(identity => identity.PublicKeyToken, StringComparer.Ordinal))
            .ToArray();

    private static void AppendIdentity(StringBuilder builder, string key, AssemblyIdentity identity)
    {
        Append(builder, key, identity.Name);
        Append(builder, $"{key}-version", identity.Version?.ToString() ?? string.Empty);
        Append(builder, $"{key}-culture", identity.Culture);
        Append(builder, $"{key}-public-key-token", identity.PublicKeyToken);
    }

    private static void Append(StringBuilder builder, string key, string value) =>
        builder.Append(key).Append('=').Append(value).Append('\n');
}