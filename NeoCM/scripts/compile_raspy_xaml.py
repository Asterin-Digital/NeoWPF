from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Compile the Raspy XAML file with NeoCM.")
    parser.add_argument(
        "--configuration",
        choices=("Debug", "Release"),
        default="Debug",
        help="NeoCM build configuration to use.",
    )
    arguments = parser.parse_args()

    neocm_root = Path(__file__).resolve().parents[1]
    raspy_root = neocm_root.parent / "Raspy"
    build_name = "x64-debug" if arguments.configuration == "Debug" else "x64-release"
    compiler_path = neocm_root / "out" / "build" / build_name / "NeoCM.exe"
    input_path = raspy_root / "Main.xaml"
    output_path = neocm_root / "artifacts" / "Raspy" / "Main.xaml.g.naml"

    if not compiler_path.is_file():
        raise FileNotFoundError(
            f"NeoCM executable not found at '{compiler_path}'. Build NeoCM first."
        )
    if not input_path.is_file():
        raise FileNotFoundError(f"Raspy XAML file not found at '{input_path}'.")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        [str(compiler_path), "compile", str(input_path), "--output", str(output_path)],
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"NeoCM failed to compile '{input_path}' with exit code {result.returncode}."
        )

    print(f"Compiled Raspy XAML: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
