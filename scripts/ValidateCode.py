#!/usr/bin/env python3
"""Syntax-check selected C/C++ files with the configured MinGW compiler."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

SOURCE_SUFFIXES = {".cpp", ".h"}
DEFAULT_MINGW_COMPILER = Path(r"E:\mingw64\bin\g++.exe")


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Syntax-check selected C/C++ files with MinGW g++.",
        epilog=(
            "Examples: python scripts/ValidateCode.py src/UI/Controls/Slider/Slider.cpp "
            "include/NeoWPF/UI/Controls/Slider/Slider.h"
        ),
    )
    parser.add_argument("files", nargs="+", type=Path, help="C/C++ files to validate")
    parser.add_argument(
        "--compiler",
        type=Path,
        help="Compiler executable. Defaults to E:\\mingw64\\bin\\g++.exe or g++ on PATH.",
    )
    parser.add_argument("--std", default="c++20", help="C++ language standard (default: c++20)")
    parser.add_argument(
        "-I",
        "--include",
        dest="include_dirs",
        action="append",
        type=Path,
        default=[],
        help="Additional include directory; may be specified multiple times",
    )
    parser.add_argument(
        "--flag",
        dest="extra_flags",
        action="append",
        default=[],
        help="Additional compiler flag; may be specified multiple times",
    )
    parser.add_argument("--quiet", action="store_true", help="Only print failures")
    return parser.parse_args()


def find_compiler(requested: Path | None) -> str:
    if requested is not None:
        return str(requested)
    if DEFAULT_MINGW_COMPILER.is_file():
        return str(DEFAULT_MINGW_COMPILER)
    compiler = shutil.which("g++")
    if compiler:
        return compiler
    raise FileNotFoundError(
        f"Could not find MinGW compiler at {DEFAULT_MINGW_COMPILER} or g++ on PATH."
    )


def resolve_files(inputs: list[Path], root: Path) -> list[Path]:
    files: list[Path] = []
    for input_path in inputs:
        path = input_path if input_path.is_absolute() else root / input_path
        path = path.resolve()
        if not path.is_file():
            raise FileNotFoundError(f"Input file does not exist: {input_path}")
        if path.suffix.lower() not in SOURCE_SUFFIXES:
            raise ValueError(f"Unsupported C/C++ file extension: {input_path}")
        files.append(path)
    return files


def validate_file(
    compiler: str,
    file_path: Path,
    root: Path,
    language_standard: str,
    include_dirs: list[Path],
    extra_flags: list[str],
) -> int:
    command = [compiler, f"-std={language_standard}", "-fsyntax-only"]
    command.extend(f"-I{path}" for path in include_dirs)
    command.extend(extra_flags)

    if file_path.suffix.lower() in {".h", ".hh", ".hpp", ".hxx"}:
        command.extend(["-x", "c++"])
    command.append(str(file_path))

    result = subprocess.run(command, cwd=root, text=True, capture_output=True)
    if result.returncode == 0:
        return 0

    print(f"FAIL {file_path.relative_to(root)}", file=sys.stderr)
    if result.stdout:
        print(result.stdout, file=sys.stderr, end="")
    if result.stderr:
        print(result.stderr, file=sys.stderr, end="")
    return result.returncode


def main() -> int:
    arguments = parse_arguments()
    root = Path(__file__).resolve().parents[1]

    try:
        compiler = find_compiler(arguments.compiler)
        files = resolve_files(arguments.files, root)
    except (FileNotFoundError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    include_dirs = [root / "include", *arguments.include_dirs]
    failures = 0
    for file_path in files:
        result = validate_file(
            compiler,
            file_path,
            root,
            arguments.std,
            include_dirs,
            arguments.extra_flags,
        )
        if result == 0:
            if not arguments.quiet:
                print(f"PASS {file_path.relative_to(root)}")
        else:
            failures += 1

    if failures:
        print(f"{failures}/{len(files)} file(s) failed validation.", file=sys.stderr)
        return 1

    if not arguments.quiet:
        print(f"Validated {len(files)} file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
