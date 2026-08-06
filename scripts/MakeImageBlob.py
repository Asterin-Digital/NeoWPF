from pathlib import Path
import pyperclip

image_path = Path(r"C:\Users\Katsu\Downloads\Kitty Cat.jpg")
header_name = "KittyCatImage"

data = image_path.read_bytes()

lines = []
lines.append("#pragma once")
lines.append("#include <cstddef>")
lines.append("")
lines.append(f"inline constexpr unsigned char {header_name}[] = {{")

for i in range(0, len(data), 12):
    chunk = data[i:i + 12]
    lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")

lines.append("};")
lines.append("")
lines.append(f"inline constexpr std::size_t {header_name}Size = sizeof({header_name});")

header = "\n".join(lines)

pyperclip.copy(header)

print("Header copied to clipboard.")