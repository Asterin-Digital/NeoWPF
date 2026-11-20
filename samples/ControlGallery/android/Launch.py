import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


PACKAGE_NAME = "com.asterin.neowpf.controlgallery"
ACTIVITY_NAME = "android.app.NativeActivity"
ANDROID_CMAKE_VERSION = "3.22.1"
PROJECT_DIR = Path(__file__).resolve().parent
REPOSITORY_DIR = PROJECT_DIR.parents[2]
ANDROID_BUILD_DIR = REPOSITORY_DIR / "out" / "build" / "android-debug-preinstalled"


def find_adb() -> str | None:
    adb_name = "adb.exe" if os.name == "nt" else "adb"
    adb = shutil.which("adb")
    if adb:
        return adb

    for variable in ("ANDROID_SDK_ROOT", "ANDROID_HOME"):
        sdk_root = os.environ.get(variable)
        if sdk_root:
            candidate = Path(sdk_root) / "platform-tools" / adb_name
            if candidate.is_file():
                return str(candidate)
    return None


def find_gradle() -> Path | None:
    wrapper_name = "gradlew.bat" if os.name == "nt" else "gradlew"
    wrapper = PROJECT_DIR / wrapper_name
    if wrapper.is_file():
        return wrapper

    gradle = shutil.which("gradle")
    return Path(gradle) if gradle else None


def find_android_build_tools(adb: str) -> tuple[str, str]:
    sdk_roots = [
        Path(os.environ[variable])
        for variable in ("ANDROID_SDK_ROOT", "ANDROID_HOME")
        if os.environ.get(variable)
    ]
    sdk_roots.append(Path(adb).resolve().parent.parent)

    cmake_name = "cmake.exe" if os.name == "nt" else "cmake"
    ninja_name = "ninja.exe" if os.name == "nt" else "ninja"
    for sdk_root in sdk_roots:
        bin_directory = sdk_root / "cmake" / ANDROID_CMAKE_VERSION / "bin"
        cmake = bin_directory / cmake_name
        ninja = bin_directory / ninja_name
        if cmake.is_file() and ninja.is_file():
            return str(cmake), str(ninja)

    cmake = shutil.which("cmake")
    ninja = shutil.which("ninja")
    if cmake and ninja:
        return cmake, ninja
    raise RuntimeError(
        f"Android CMake {ANDROID_CMAKE_VERSION} and Ninja were not found in the SDK or PATH."
    )


def make_command(program: Path | str, *arguments: str) -> list[str]:
    command = str(program)
    if os.name == "nt" and Path(command).suffix.lower() in {".bat", ".cmd"}:
        return [os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", command, *arguments]
    return [command, *arguments]


def get_device_serial(adb: str, requested_serial: str | None) -> str:
    result = subprocess.run(
        [adb, "devices"],
        check=True,
        capture_output=True,
        text=True,
    )
    devices = {
        fields[0]: fields[1]
        for line in result.stdout.splitlines()[1:]
        if len(fields := line.split()) >= 2
    }
    available = [serial for serial, state in devices.items() if state == "device"]

    if requested_serial:
        if requested_serial not in available:
            raise RuntimeError(f"ADB device '{requested_serial}' is not connected and ready.")
        return requested_serial

    if len(available) == 1:
        return available[0]
    if not available:
        raise RuntimeError("No connected ADB devices are ready.")
    raise RuntimeError(
        "Multiple ADB devices are connected. Select one with --serial: "
        + ", ".join(available)
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build the Control Gallery APK, install it, and launch it on an ADB device."
    )
    parser.add_argument("--serial", help="ADB device serial when more than one device is connected.")
    args = parser.parse_args()

    adb = find_adb()
    gradle = find_gradle()
    if not adb:
        parser.error("adb was not found on PATH or under ANDROID_SDK_ROOT/ANDROID_HOME.")
    if not gradle:
        parser.error("Gradle was not found on PATH and no Gradle wrapper exists in this project.")

    try:
        serial = get_device_serial(adb, args.serial)
        cmake, ninja = find_android_build_tools(adb)
    except RuntimeError as error:
        parser.error(str(error))

    subprocess.run(
        [
            sys.executable,
            str(REPOSITORY_DIR / "private" / "Scripts" / "Build.py"),
            "--source",
            str(REPOSITORY_DIR),
            "--build",
            str(ANDROID_BUILD_DIR),
            "--config",
            "Debug",
            "--android",
            "--cmake",
            cmake,
            "--target",
            "NeoWPF",
            "-D",
            "VCPKG_MANIFEST_MODE=OFF",
            "-D",
            f"CMAKE_MAKE_PROGRAM={ninja}",
        ],
        cwd=REPOSITORY_DIR,
        check=True,
    )

    subprocess.run(
        make_command(gradle, "assembleDebug"),
        cwd=PROJECT_DIR,
        check=True,
    )

    apk_directory = PROJECT_DIR / "build" / "outputs" / "apk" / "debug"
    apks = list(apk_directory.glob("*.apk"))
    if len(apks) != 1:
        raise RuntimeError(
            f"Expected one debug APK in '{apk_directory}', found {len(apks)}."
        )

    device_adb = [adb, "-s", serial]
    subprocess.run([*device_adb, "install", "-r", str(apks[0])], check=True)
    subprocess.run(
        [*device_adb, "shell", "am", "force-stop", PACKAGE_NAME],
        check=True,
    )
    subprocess.run(
        [
            *device_adb,
            "shell",
            "am",
            "start",
            "-n",
            f"{PACKAGE_NAME}/{ACTIVITY_NAME}",
        ],
        check=True,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
