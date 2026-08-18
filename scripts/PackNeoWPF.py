import sys
import zipfile
from pathlib import Path

ROOT = Path(r"C:\Users\Katsu\source\repos\NeoWPF")

INCLUDE_DIR = ROOT / "include" / "NeoWPF"
LICENSE = ROOT / "LICENSE"
OUTPUT_DIR = ROOT / "output"

mode = "Release" if len(sys.argv) == 2 and sys.argv[1] == "--release" else "Debug"

if len(sys.argv) > 2 or (len(sys.argv) == 2 and sys.argv[1] != "--release"):
    print("Usage: PackNeoWPF.py [--release]")
    sys.exit(1)

LIB = ROOT / "x64" / mode / "NeoWPF.lib"
OUTPUT = OUTPUT_DIR / mode
ZIP = OUTPUT / f"NeoWPF-{mode}.zip"

if not INCLUDE_DIR.is_dir():
    print(f"Missing include directory: {INCLUDE_DIR}")
    sys.exit(1)

if not LIB.is_file():
    print(f"Missing library: {LIB}")
    sys.exit(1)

if not LICENSE.is_file():
    print(f"Missing LICENSE: {LICENSE}")
    sys.exit(1)

OUTPUT.mkdir(parents=True, exist_ok=True)

with zipfile.ZipFile(ZIP, "w", zipfile.ZIP_DEFLATED) as archive:
    for file in INCLUDE_DIR.rglob("*"):
        if file.is_file():
            archive.write(
                file,
                Path("include") / "NeoWPF" / file.relative_to(INCLUDE_DIR)
            )

    archive.write(LIB, Path("lib") / "NeoWPF.lib")
    archive.write(LICENSE, "LICENSE")

print(f"Packed NeoWPF {mode} SDK")
print(f"Output: {ZIP}")