import os
import shutil
import subprocess
import sys
import time
from pathlib import Path
 
configuration = "Release" if "--release" in sys.argv else "Debug"

open_saphire_after_build = False

source_root = Path(__file__).resolve().parents[1]
dest_root = source_root.parent / "Saphire"
build_root = source_root / "x64" / configuration
preset_name = f"x64-{configuration.lower()}"


def load_msvc_environment() -> None:
    if os.environ.get("VCToolsInstallDir"):
        return

    visual_studio_roots = [
        Path(value) / "Microsoft Visual Studio"
        for value in (os.environ.get("ProgramFiles"), os.environ.get("ProgramFiles(x86)"))
        if value
    ]
    dev_command = next(
        (
            path
            for root in visual_studio_roots
            for path in root.glob("**/Common7/Tools/VsDevCmd.bat")
            if "\\2022\\" in str(path)
        ),
        None,
    )
    if dev_command is None:
        raise FileNotFoundError("Visual Studio 2022 developer environment was not found")

    result = subprocess.run(
        f'call "{dev_command}" -arch=x64 -host_arch=x64 && set',
        shell=True,
        capture_output=True,
        text=True,
        check=True,
    )
    for line in result.stdout.splitlines():
        if "=" in line:
            name, value = line.split("=", 1)
            os.environ[name] = value


load_msvc_environment()

print(f"Building NeoWPF ({configuration})...")

subprocess.run(
    [
        "cmake",
        "--preset",
        preset_name,
        f"-DCMAKE_BUILD_TYPE={configuration}",
    ],
    cwd=source_root,
    check=True,
)

subprocess.run(
    [
        "cmake",
        "--build",
        "--preset",
        preset_name,
    ],
    cwd=source_root,
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