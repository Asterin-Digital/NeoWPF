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

print("\nCopying library...")

if not source_lib.exists():
    raise FileNotFoundError(source_lib)

dest_lib.parent.mkdir(parents=True, exist_ok=True)
shutil.copy2(source_lib, dest_lib)

print("Library copied.")
print("\nDone!")