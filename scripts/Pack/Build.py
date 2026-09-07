import os 
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

def DoFileRead(file):
    if os.path.isfile(file):
        with open(file, 'r') as f:
            contents = f.read()
            return contents
    else:
        return None

def RunCMake(root_dir, preset):
    configure = subprocess.run(
        ["cmake", "--preset", preset],
        cwd=root_dir,
        capture_output=True,
        text=True,
    )
    if configure.returncode != 0:
        print("CMake configure failed")
        print(configure.stdout)
        print(configure.stderr)
        return False

    print("All checks passed, running build")
    build = subprocess.run(
        ["cmake", "--build", "--preset", preset],
        cwd=root_dir,
        capture_output=True,
        text=True,
    )
    if build.returncode == 0:
        print("Build succeeded")
        return True

    print("Build failed")
    print(build.stdout)
    print(build.stderr)
    return False

def ParseArguments(args=None):
    parser = argparse.ArgumentParser()

    if args is None:
        args = sys.argv[1:]
    if not args:
        parser.error("a platform or configuration argument is required")

    platform = parser.add_mutually_exclusive_group()
    platform.add_argument(
        "--linux",
        dest="platform",
        action="store_const",
        const="linux",
    )
    platform.add_argument(
        "--windows",
        dest="platform",
        action="store_const",
        const="windows",
    )

    configuration = parser.add_mutually_exclusive_group()
    configuration.add_argument(
        "--debug",
        dest="configuration",
        action="store_const",
        const="debug",
    )
    configuration.add_argument(
        "--release",
        dest="configuration",
        action="store_const",
        const="release",
    )

    parsed = parser.parse_args(args)
    platform_name = parsed.platform or "windows"
    configuration_name = parsed.configuration or "debug"
    preset_platform = "x64" if platform_name == "windows" else "linux"
    return f"{preset_platform}-{configuration_name}"

def Main(args=None):
    print("Started")

    preset = ParseArguments(args)
    print("Selected preset", preset)

    root_dir = Path(__file__).resolve().parents[2]
    print("Running from", root_dir)

    if not os.path.isfile(os.path.join(root_dir, "CMakeLists.txt")):
        print("could not find CMakeLists file, are you running from the correct directory?")
        return 1

    cmake = shutil.which("cmake")
    if not cmake:
        print("Could not find CMake on path")
        return 1

    build = RunCMake(root_dir, preset)

    return 0 if build else 1


if __name__ == "__main__":
    Main()