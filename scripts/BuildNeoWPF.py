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

vcpkg_root = os.environ.get("VCPKG_ROOT")
if not vcpkg_root:
    vcpkg_candidates = [
        Path("C:/vcpkg"),
        Path(os.environ.get("LOCALAPPDATA", "")) / "vcpkg",
    ]
    vcpkg_root = next(
        (str(path) for path in vcpkg_candidates if (path / "scripts" / "buildsystems" / "vcpkg.cmake").is_file()),
        None,
    )

msvc_tools_dir = Path(os.environ["VCToolsInstallDir"])
msvc_bin_dir = msvc_tools_dir / "bin" / "Hostx64" / "x64"
msvc_compiler = msvc_bin_dir / "cl.exe"
msvc_linker = msvc_bin_dir / "link.exe"

if not msvc_compiler.is_file() or not msvc_linker.is_file():
    raise FileNotFoundError(f"MSVC compiler or linker not found under {msvc_bin_dir}")

print(f"Building NeoWPF ({configuration})...")

subprocess.run(
    [
        "cmake",
        "--preset",
        preset_name,
        f"-DCMAKE_CXX_COMPILER={msvc_compiler}",
        f"-DCMAKE_LINKER={msvc_linker}",
        f"-DCMAKE_BUILD_TYPE={configuration}",
        "-DVCPKG_TARGET_TRIPLET=x64-windows-static-md",
        *([f"-DCMAKE_TOOLCHAIN_FILE={Path(vcpkg_root) / 'scripts' / 'buildsystems' / 'vcpkg.cmake'}"] if vcpkg_root else []),
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