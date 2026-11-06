import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
ANDROID_PROJECT = ROOT / "samples" / "Android"
BUILD_DIR = ROOT / "NWpfBuild" / "Android"
APK = ANDROID_PROJECT / "app" / "build" / "outputs" / "apk" / "debug" / "app-debug.apk"


def run(command: list[str | Path], cwd: Path | None = None) -> int:
    try:
        return subprocess.run(command, cwd=cwd, check=False).returncode
    except FileNotFoundError:
        print(f"InstallDroidApp.py: command not found: {command[0]}", file=sys.stderr)
        return 127


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build and install the NeoWPF Android sample app."
    )
    parser.add_argument(
        "--android-ndk",
        type=Path,
        help="Android NDK directory; defaults to Build.py environment discovery.",
    )
    arguments = parser.parse_args()

    build_command: list[str | Path] = [
        sys.executable,
        ROOT / "private" / "Scripts" / "Build.py",
        "--source",
        ROOT,
        "--android",
        "--build",
        BUILD_DIR,
        "--target",
        "NeoWPF",
    ]
    if arguments.android_ndk:
        build_command.extend(["--android-ndk", arguments.android_ndk])

    result = run(build_command, cwd=ROOT)
    if result != 0:
        return result

    gradle = shutil.which("gradle")
    if gradle is None:
        print("InstallDroidApp.py: Gradle was not found on PATH.", file=sys.stderr)
        return 127

    gradle_command: list[str | Path] = [
        gradle, ":app:assembleDebug", "--no-daemon"
    ]
    if os.name == "nt" and Path(gradle).suffix.lower() in (".bat", ".cmd"):
        gradle_command = [
            os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", *gradle_command
        ]
    result = run(gradle_command, cwd=ANDROID_PROJECT)
    if result != 0:
        return result

    if not APK.is_file():
        print(f"InstallDroidApp.py: APK was not created: {APK}", file=sys.stderr)
        return 1

    result = run(["adb", "install", "-r", APK])
    if result == 0:
        print(f"Installed {APK}")
    return result


if __name__ == "__main__":
    raise SystemExit(main())