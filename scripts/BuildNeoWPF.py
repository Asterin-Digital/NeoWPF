import shutil
import subprocess
import sys
import time
from pathlib import Path
 
configuration = "Release" if "--release" in sys.argv else "Debug"

open_saphire_after_build = False

source_root = Path(r"C:\Users\Katsu\source\repos\NeoWPF")
dest_root = Path(r"C:\Users\Katsu\source\repos\Saphire")
build_root = source_root / "x64" / configuration

print(f"Building NeoWPF ({configuration})...")

subprocess.run(
    [
        "MSBuild.exe",
        source_root / "NeoWPF.slnx",
        "-target:NeoWPF",
        "/m",
        "/restore",
        f"/p:Configuration={configuration}",
        "/p:Platform=x64",
    ],
    check=True,
)

source_lib = build_root / "NeoWPF.lib"
dest_lib = dest_root / "libs" / "NeoWPF.lib"

print("\nCopying library...")

if not source_lib.exists():
    raise FileNotFoundError(source_lib)

dest_lib.parent.mkdir(parents=True, exist_ok=True)
shutil.copy2(source_lib, dest_lib)

print("Library copied.")

if open_saphire_after_build:
    time.sleep(1)

    saphire_exe = build_root / "Saphire.exe"

    print(f"Launching: {saphire_exe}")

    subprocess.run(
        [str(saphire_exe)],
        cwd=build_root,
        check=True,
    )

print("\nDone!")