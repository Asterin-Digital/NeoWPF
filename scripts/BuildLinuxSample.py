import shlex
import subprocess
import sys
from pathlib import Path


def run_wsl(source_root: Path, command: str) -> None:
    result = subprocess.run(
        ["wsl.exe", "--", "bash", "-lc", command],
        cwd=source_root,
    )
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def main() -> int:
    source_root = Path(__file__).resolve().parents[1]
    configuration = "Debug"
    preset = "linux-debug"

    sample_source = source_root / "samples" / "LinuxBasicApp.cpp"
    library = source_root / "x64" / configuration / "libNeoWPF.a"
    output = source_root / "out" / "linux" / configuration / "NeoWPF-LinuxBasicApp"

    if not sample_source.is_file():
        print(f"error: sample source not found: {sample_source}", file=sys.stderr)
        return 1

    try:
        windows_path = str(source_root).replace("\\", "/")

        wsl_root = subprocess.check_output(
            [
                "wsl.exe",
                "--",
                "wslpath",
                "-a",
                windows_path,
            ],
            text=True,
        ).strip()
    except (FileNotFoundError, subprocess.CalledProcessError) as error:
        print(f"error: unable to locate WSL: {error}", file=sys.stderr)
        return 1

    root = shlex.quote(wsl_root)

    command_parts = [
        "set -e",
        f"ROOT={root}",
        f"CONFIG={shlex.quote(configuration)}",
        "export ROOT CONFIG",
        f"cmake --preset {shlex.quote(preset)}",
        f'cmake --build --preset {shlex.quote(preset)} --config "$CONFIG" --target NeoWPF',
        'mkdir -p "$ROOT/out/linux/$CONFIG"',
        'g++ -std=c++20'
        ' -I"$ROOT/include"'
        ' "$ROOT/samples/LinuxBasicApp.cpp"'
        ' "$ROOT/x64/$CONFIG/libNeoWPF.a"'
        ' -lvulkan'
        ' $(pkg-config --libs wayland-client freetype2 harfbuzz fontconfig)'
        ' -pthread'
        ' -o "$ROOT/out/linux/$CONFIG/NeoWPF-LinuxBasicApp"',
        '"$ROOT/out/linux/$CONFIG/NeoWPF-LinuxBasicApp"',
    ]

    command = "\n".join(command_parts)

    print(f"Building NeoWPF Linux sample ({configuration}) through WSL...")
    run_wsl(source_root, command)

    if not library.is_file():
        print(
            f"warning: expected archive was not found at {library}",
            file=sys.stderr,
        )

    print(f"Linux sample: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
