namespace NeoWPF.DotNet.Bridge.Platform;

internal static class UnmanagedLibraryResolver
{
    public static string? ResolveAdjacent(string assemblyPath, string libraryName)
    {
        var directory = Path.GetDirectoryName(assemblyPath);
        if (directory is null)
            return null;

        string[] extensions;
        string[] prefixes;
        if (OperatingSystem.IsWindows())
        {
            extensions = [".dll"];
            prefixes = [string.Empty];
        }
        else if (OperatingSystem.IsLinux())
        {
            extensions = [".so"];
            prefixes = [string.Empty, "lib"];
        }
        else
        {
            return null;
        }

        foreach (var prefix in prefixes)
        foreach (var extension in extensions)
        {
            var candidateName = libraryName.EndsWith(extension, StringComparison.OrdinalIgnoreCase)
                ? libraryName
                : $"{prefix}{libraryName}{extension}";
            var candidatePath = Path.Combine(directory, candidateName);
            if (File.Exists(candidatePath))
                return candidatePath;
        }
        return null;
    }
}