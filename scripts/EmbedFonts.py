import argparse
from pathlib import Path

ALLOWED_EXTENSIONS = {
    ".ttf",
    ".otf",
    ".ttc",
    ".otc",
    ".woff",
    ".woff2",
}


def sanitize_symbol(value):
    cleaned = "".join(ch if ch.isalnum() else "_" for ch in value)
    if not cleaned:
        return "embedded_font"
    if cleaned[0].isdigit():
        cleaned = "_" + cleaned
    return cleaned


def emit_byte_array(name, blob):
    lines = [f"static constexpr unsigned char {name}[] = {{"]
    for index in range(0, len(blob), 12):
        chunk = blob[index : index + 12]
        hex_values = ", ".join(f"0x{byte:02x}" for byte in chunk)
        lines.append(f"    {hex_values},")
    lines.append("};")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Embed all fonts from a directory into a hidden C++ source file and public header."
    )
    parser.add_argument(
        "input_dir",
        nargs="?",
        default=str(Path(__file__).resolve().parents[1] / "private" / "Embedded"),
        help="Directory containing font files to embed.",
    )
    args = parser.parse_args()

    source_dir = Path(args.input_dir).resolve()
    if not source_dir.exists() or not source_dir.is_dir():
        raise SystemExit(f"Embedded font directory not found: {source_dir}")

    font_files = sorted(
        [path for path in source_dir.iterdir() if path.is_file() and path.suffix.lower() in ALLOWED_EXTENSIONS],
        key=lambda path: path.name.lower(),
    )
    if not font_files:
        raise SystemExit(f"No font files found in {source_dir}")

    repo_root = Path(__file__).resolve().parents[1]
    output_src_dir = repo_root / "src" / "Core" / "Application" / "Resources" / "Embedded"
    output_include_dir = repo_root / "include" / "NeoWPF" / "Core" / "Application" / "Resources" / "Embedded"
    output_src_dir.mkdir(parents=True, exist_ok=True)
    output_include_dir.mkdir(parents=True, exist_ok=True)

    header_path = output_include_dir / "FallbackFont.h"
    cpp_path = output_src_dir / "FallbackFont.cpp"

    records = []
    for font_path in font_files:
        blob = font_path.read_bytes()
        symbol_name = sanitize_symbol(font_path.stem)
        records.append(
            {
                "symbol": symbol_name,
                "name": font_path.stem,
                "blob": blob,
            }
        )

    source_lines = [
        "#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>",
        "",
        "namespace {",
    ]

    for record in records:
        source_lines.append(emit_byte_array(f"g_embedded_font_{record['symbol']}", record["blob"]))
        source_lines.append("")

    source_lines.append("struct EmbeddedFontRecord")
    source_lines.append("{")
    source_lines.append("    const char* name = nullptr;")
    source_lines.append("    const unsigned char* data = nullptr;")
    source_lines.append("    std::size_t size = 0;")
    source_lines.append("};")
    source_lines.append("")
    source_lines.append("constexpr EmbeddedFontRecord kEmbeddedFallbackFonts[] = {")
    for record in records:
        source_lines.append(
            f"    {{ \"{record['name']}\", g_embedded_font_{record['symbol']}, sizeof(g_embedded_font_{record['symbol']}) }},"
        )
    source_lines.append("};")
    source_lines.append("}")
    source_lines.append("")
    source_lines.append("namespace NeoWPF::Core::Application::Resources::Embedded {")
    source_lines.append("std::size_t GetEmbeddedFallbackFontCount() noexcept")
    source_lines.append("{")
    source_lines.append("    return sizeof(kEmbeddedFallbackFonts) / sizeof(kEmbeddedFallbackFonts[0]);")
    source_lines.append("}")
    source_lines.append("")
    source_lines.append("const char* GetEmbeddedFallbackFontName(std::size_t index) noexcept")
    source_lines.append("{")
    source_lines.append("    if (index >= GetEmbeddedFallbackFontCount()) {")
    source_lines.append("        return nullptr;")
    source_lines.append("    }")
    source_lines.append("    return kEmbeddedFallbackFonts[index].name;")
    source_lines.append("}")
    source_lines.append("")
    source_lines.append("bool TryGetEmbeddedFallbackFont(std::size_t index, FontRecord& outRecord) noexcept")
    source_lines.append("{")
    source_lines.append("    if (index >= GetEmbeddedFallbackFontCount()) {")
    source_lines.append("        return false;")
    source_lines.append("    }")
    source_lines.append("    outRecord.name = kEmbeddedFallbackFonts[index].name;")
    source_lines.append("    outRecord.data = kEmbeddedFallbackFonts[index].data;")
    source_lines.append("    outRecord.size = kEmbeddedFallbackFonts[index].size;")
    source_lines.append("    return true;")
    source_lines.append("}")
    source_lines.append("}")
    source_lines.append("")

    cpp_path.write_text("\n".join(source_lines) + "\n", encoding="utf-8")

    header_lines = [
        "#pragma once",
        "",
        "#include <cstddef>",
        "",
        "namespace NeoWPF::Core::Application::Resources::Embedded {",
        "",
        "struct FontRecord {",
        "    const char* name = nullptr;",
        "    const unsigned char* data = nullptr;",
        "    std::size_t size = 0;",
        "};",
        "",
        "std::size_t GetEmbeddedFallbackFontCount() noexcept;",
        "const char* GetEmbeddedFallbackFontName(std::size_t index) noexcept;",
        "bool TryGetEmbeddedFallbackFont(std::size_t index, FontRecord& outRecord) noexcept;",
        "",
        "}",
        "",
    ]
    header_path.write_text("\n".join(header_lines), encoding="utf-8")

    print(f"Embedded {len(font_files)} font file(s) from {source_dir}")
    print(f"Generated {header_path}")
    print(f"Generated {cpp_path}")


if __name__ == "__main__":
    main()
