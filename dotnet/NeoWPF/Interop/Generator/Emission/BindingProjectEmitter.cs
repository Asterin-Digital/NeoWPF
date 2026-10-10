using System.Text;
using System.Xml;
using NeoWPF.DotNet.Interop.Generator.AssemblyInspection;
using NeoWPF.DotNet.Interop.Generator.Binding;

namespace NeoWPF.DotNet.Interop.Generator.Emission;

internal static class BindingProjectEmitter
{
    public static string Emit(BindingModel model, IReadOnlyList<AssemblyIdentity> identities)
    {
        var output = new StringBuilder();
        using var writer = XmlWriter.Create(output, new XmlWriterSettings
        {
            Indent = true,
            NewLineChars = "\n",
            OmitXmlDeclaration = true,
        });

        writer.WriteStartElement("Project");
        writer.WriteAttributeString("Sdk", "Microsoft.NET.Sdk");
        writer.WriteStartElement("PropertyGroup");
        WriteElement(writer, "TargetFramework", "net10.0");
        WriteElement(writer, "ImplicitUsings", "enable");
        WriteElement(writer, "Nullable", "enable");
        WriteElement(writer, "EnableDefaultCompileItems", "false");
        WriteElement(writer, "AssemblyName", GetBindingAssemblyName(model.Name));
        WriteElement(writer, "RootNamespace", model.Name);
        writer.WriteEndElement();

        writer.WriteStartElement("ItemGroup");
        WriteElement(writer, "Compile", null, ("Include", $"{model.Name}.Contract.g.cs"));
        foreach (var identity in identities)
        {
            writer.WriteStartElement("Reference");
            writer.WriteAttributeString("Include", identity.Name);
            WriteElement(writer, "HintPath", identity.Location);
            writer.WriteEndElement();
        }
        writer.WriteEndElement();
        writer.WriteEndElement();
        writer.Flush();
        return output.ToString();
    }

    public static string GetBindingAssemblyName(string bindingName) => $"{bindingName}.Bindings";

    private static void WriteElement(XmlWriter writer, string name, string? value, (string Name, string Value)? attribute = null)
    {
        writer.WriteStartElement(name);
        if (attribute is { } item)
            writer.WriteAttributeString(item.Name, item.Value);
        if (value is not null)
            writer.WriteString(value);
        writer.WriteEndElement();
    }
}