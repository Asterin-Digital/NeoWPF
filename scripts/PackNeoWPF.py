import sys
import shutil
import zipfile
from pathlib import Path

ROOT = Path(r"C:\Users\Katsu\source\repos\NeoWPF")

INCLUDE = ROOT / "include" / "NeoWPF"
LICENSE = ROOT / "LICENSE"

MODE = "Release" if len(sys.argv) > 1 and sys.argv[1] == "--release" else "Debug"

if len(sys.argv) > 2 or (len(sys.argv) == 2 and sys.argv[1] != "--release"):
    print("Usage: PackNeoWPF.py [--release]")
    sys.exit(1)

LIB = ROOT / "x64" / MODE / "NeoWPF.lib"
OUTPUT = ROOT / "output" / MODE
ZIP = OUTPUT / f"NeoWPF-{MODE}.zip"

if OUTPUT.exists():
    shutil.rmtree(OUTPUT)

OUTPUT.mkdir(parents=True)

with zipfile.ZipFile(ZIP, "w", zipfile.ZIP_DEFLATED) as z:
    for file in INCLUDE.rglob("*"):
        if file.is_file():
            z.write(
                file,
                Path("include") / "NeoWPF" / file.relative_to(INCLUDE)
            )

    z.write(LIB, Path("lib") / "NeoWPF.lib")
    z.write(LICENSE, "LICENSE")

print(f"Packed NeoWPF {MODE} SDK")
print(ZIP)