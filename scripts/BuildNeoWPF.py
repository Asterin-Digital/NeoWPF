import shutil
import subprocess
import sys
from pathlib import Path

configuration = "Release" if "--release" in sys.argv else "Debug"

print(f"Building NeoWPF ({configuration})...")

subprocess.run(
    [
        "MSBuild.exe",
        r"C:\Users\Katsu\source\repos\NeoWPF\NeoWPF.slnx",
        "-target:NeoWPF",
        "/m",
        "/restore",
        f"/p:Configuration={configuration}",
        "/p:Platform=x64",
    ],
    check=True,
)

source_root = Path(r"C:\Users\Katsu\source\repos\NeoWPF")
dest_root = Path(r"C:\Users\Katsu\source\repos\Saphire")

source_lib = source_root / "x64" / configuration / "NeoWPF.lib"
dest_lib = dest_root / "libs" / "NeoWPF.lib"

source_include = source_root / "include" / "NeoWPF"
dest_include = dest_root / "include" / "NeoWPF"

print("\nCopying library...")

if not source_lib.exists():
    raise FileNotFoundError(source_lib)

dest_lib.parent.mkdir(parents=True, exist_ok=True)
shutil.copy2(source_lib, dest_lib)

print("Library copied.")

print("\nCopying include folder...")

if not source_include.exists():
    raise FileNotFoundError(source_include)

if dest_include.exists():
    shutil.rmtree(dest_include)

shutil.copytree(source_include, dest_include)

comment = (
    "// This is not the canonical {name} from NeoWPF!!\n"
    "// Please do NOT update this header file since it's automatically updated\n\n"
)

header_extensions = {".h", ".hpp", ".hxx", ".hh", ".h++"}
count = 0

for file in dest_include.rglob("*"):
    if file.suffix.lower() in header_extensions:
        original = file.read_text(encoding="utf-8")
        file.write_text(
            comment.format(name=file.name) + original,
            encoding="utf-8",
            newline="\n",
        )
        count += 1

print("\nDone!")