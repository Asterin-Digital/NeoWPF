using NeoWPF.DotNet.Interop.Generator.Binding;
using NeoWPF.DotNet.Interop.Generator.Diagnostics;

namespace NeoWPF.DotNet.Interop.Generator.Parsing;

public enum BindingTokenKind
{
    Identifier,
    String,
    True,
    False,
    Symbol,
    End,
}

public readonly record struct BindingToken(
    BindingTokenKind Kind,
    string Text,
    SourceLocation Location,
    bool IsEscapedIdentifier = false);

public sealed class BindingLexer
{
    private readonly string _text;
    private readonly string _file;
    private int _offset;
    private int _line = 1;
    private int _column = 1;

    public BindingLexer(string text, string file = "<input>")
    {
        _text = text;
        _file = file;
    }

    public IReadOnlyList<BindingToken> Lex()
    {
        var tokens = new List<BindingToken>();
        while (!AtEnd)
        {
            if (char.IsWhiteSpace(Current))
            {
                Advance();
                continue;
            }

            if (Current == '/' && Peek() == '/')
            {
                SkipLineComment();
                continue;
            }

            if (Current == '/' && Peek() == '*')
            {
                SkipBlockComment();
                continue;
            }

            var location = Location;
            if (Current == '"')
            {
                tokens.Add(new BindingToken(BindingTokenKind.String, ReadString(location), location));
                continue;
            }

            if (IsIdentifierStart(Current) || Current == '@')
            {
                var value = ReadIdentifier();
                var isEscapedIdentifier = value[0] == '@';
                var kind = value switch
                {
                    "true" => BindingTokenKind.True,
                    "false" => BindingTokenKind.False,
                    _ => BindingTokenKind.Identifier,
                };
                tokens.Add(new BindingToken(kind, value, location, isEscapedIdentifier));
                continue;
            }

            if ("{}();,.<>?[]:*".Contains(Current))
            {
                tokens.Add(new BindingToken(BindingTokenKind.Symbol, Current.ToString(), location));
                Advance();
                continue;
            }

            throw Error(BindingDiagnosticIds.UnexpectedToken, $"Unexpected character '{Current}'.");
        }

        tokens.Add(new BindingToken(BindingTokenKind.End, string.Empty, Location));
        return tokens;
    }

    private string ReadIdentifier()
    {
        var start = _offset;
        if (Current == '@') Advance();
        if (!IsIdentifierStart(Current)) throw Error(BindingDiagnosticIds.ExpectedIdentifier, "Expected an identifier after '@'.");
        Advance();
        while (!AtEnd && IsIdentifierPart(Current)) Advance();
        return _text[start.._offset];
    }

    private string ReadString(SourceLocation location)
    {
        Advance();
        var result = new System.Text.StringBuilder();
        while (!AtEnd && Current != '"')
        {
            if (Current == '\n' || Current == '\r') throw ErrorAt(BindingDiagnosticIds.NewlineInString, "String literals cannot contain a newline.", location);
            if (Current != '\\')
            {
                result.Append(Current);
                Advance();
                continue;
            }

            Advance();
            if (AtEnd) throw ErrorAt(BindingDiagnosticIds.UnterminatedEscape, "Unterminated escape sequence.", location);
            var escaped = Current;
            result.Append(escaped switch
            {
                '\\' => '\\',
                '"' => '"',
                'n' => '\n',
                't' => '\t',
                'u' => ReadUnicodeEscape(),
                _ => throw Error(BindingDiagnosticIds.UnsupportedEscape, $"Unsupported escape '\\{escaped}'."),
            });
            if (escaped != 'u') Advance();
        }
        if (AtEnd) throw ErrorAt(BindingDiagnosticIds.UnterminatedString, "Unterminated string literal.", location);
        Advance();
        return result.ToString();
    }

    private char ReadUnicodeEscape()
    {
        Advance();
        if (_offset + 4 > _text.Length) throw Error(BindingDiagnosticIds.InvalidUnicodeEscape, "Unicode escape must contain four hexadecimal digits.");
        var hex = _text.Substring(_offset, 4);
        if (!ushort.TryParse(hex, System.Globalization.NumberStyles.AllowHexSpecifier, null, out var value))
            throw Error(BindingDiagnosticIds.InvalidUnicodeEscape, "Unicode escape must contain four hexadecimal digits.");
        for (var index = 0; index < 4; index++) Advance();
        return (char)value;
    }

    private void SkipLineComment()
    {
        while (!AtEnd && Current != '\n') Advance();
    }

    private void SkipBlockComment()
    {
        Advance();
        Advance();
        while (!AtEnd)
        {
            if (Current == '*' && Peek() == '/')
            {
                Advance();
                Advance();
                return;
            }
            Advance();
        }
        throw Error(BindingDiagnosticIds.UnterminatedBlockComment, "Unterminated block comment.");
    }

    private bool AtEnd => _offset >= _text.Length;
    private char Current => AtEnd ? '\0' : _text[_offset];
    private char Peek() => _offset + 1 >= _text.Length ? '\0' : _text[_offset + 1];
    private SourceLocation Location => new(_file, _line, _column);
    private static bool IsIdentifierStart(char value) =>
        value is >= 'A' and <= 'Z' or >= 'a' and <= 'z' or '_';

    private static bool IsIdentifierPart(char value) =>
        IsIdentifierStart(value) || value is >= '0' and <= '9';

    private void Advance()
    {
        if (AtEnd) return;
        if (_text[_offset++] == '\n')
        {
            _line++;
            _column = 1;
        }
        else
        {
            _column++;
        }
    }

    private BindingDiagnosticException Error(string id, string message) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, Location));

    private static BindingDiagnosticException ErrorAt(string id, string message, SourceLocation location) =>
        new(new BindingDiagnostic(id, BindingDiagnosticSeverity.Error, message, location));
}
