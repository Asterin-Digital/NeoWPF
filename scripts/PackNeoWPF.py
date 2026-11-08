from __future__ import annotations

import argparse
import os
import re
import sys
import zipfile
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
BUILD_ROOT = ROOT / "NWpfBuild"
VERSION_FILE = ROOT / "NeoWPFVersion.txt"
LICENSE_FILE = ROOT / "LICENSE"
INCLUDE_ROOT = ROOT / "include"
PACKAGE_ROOT = BUILD_ROOT / "Packed"

PLATFORMS = {
    "win32": {
        "folder": "Windows",
        "file_platform": "Windows",
        "architecture": "x64",
        "package_platform": "windows",
        "extension": ".lib",
    },
    "linux": {
        "folder": "Linux",
        "file_platform": "Linux",
        "architecture": "x64",
        "package_platform": "linux",
        "extension": ".a",
    },
    "android": {
        "folder": "Android",
        "file_platform": "Android",
        "architecture": "arm64",
        "package_platform": "android",
        "extension": ".a",
    },
}

CONFIGS = ("Release", "Debug")
VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()

    parser.add_argument("--all", action="store_true")
    parser.add_argument("--win32", action="store_true")
    parser.add_argument("--linux", action="store_true")
    parser.add_argument("--android", action="store_true")
    parser.add_argument(
        "--combined",
        action="store_true",
    )

    parser.add_argument("--release", action="store_true")
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--both", action="store_true")

    args = parser.parse_args()

    if not (args.all or args.win32 or args.linux or args.android or args.combined):
        parser.error("you need a platform arg: --all, --win32, --linux, or --android")

    config_args = sum((args.release, args.debug, args.both))
    if config_args > 1:
        parser.error("use only one of --release, --debug, or --both")

    return args


def read_version() -> str:
    version_file = os.environ.get("NEOWPF_SHARED_VERSION_FILE")
    if version_file:
        source_version_file = Path(version_file)
    elif sys.platform.startswith("linux") and "/.vs/" in str(ROOT):
        candidates = list(
            Path("/mnt/c/Users").glob(
                "*/source/repos/NeoWPF/NeoWPFVersion.txt"
            )
        )
        if len(candidates) != 1:
            raise ValueError(
                "could not uniquely locate the shared Windows NeoWPFVersion.txt; "
                "set NEOWPF_SHARED_VERSION_FILE"
            )
        source_version_file = candidates[0]
    else:
        source_version_file = VERSION_FILE

    def read_valid_version(path: Path) -> str:
        if not path.is_file():
            raise FileNotFoundError(f"couldn't find version file: {path}")
        version = path.read_text(encoding="utf-8").strip()
        if VERSION_PATTERN.fullmatch(version) is None:
            raise ValueError(f"{path} must contain Major.Minor.Patch")
        return version

    return read_valid_version(source_version_file)


def selected_platforms(args: argparse.Namespace) -> list[str]:
    if args.win32 or args.linux or args.android:
        platforms = []
        if args.win32:
            platforms.append("win32")
        if args.linux:
            platforms.append("linux")
        if args.android:
            platforms.append("android")
        return platforms

    return list(PLATFORMS)


def selected_configs(args: argparse.Namespace) -> list[str]:
    if args.release:
        return ["Release"]
    if args.debug:
        return ["Debug"]

    return list(CONFIGS)


def find_library(platform_key: str, config: str, version: str) -> Path | None:
    info = PLATFORMS[platform_key]
    filename = info.get(
        "library_name",
        f"NeoWPF.{info['file_platform']}.{config}.{version}{info['extension']}",
    )

    candidate = BUILD_ROOT / info["folder"] / config / filename
    return candidate if candidate.is_file() else None


def validate_package_files() -> None:
    if not LICENSE_FILE.is_file():
        raise FileNotFoundError(f"couldn't find LICENSE: {LICENSE_FILE}")

    for include_name in ("NeoWPF", "NeoNaml"):
        include_dir = INCLUDE_ROOT / include_name
        if not include_dir.is_dir():
            raise FileNotFoundError(f"couldn't find include folder: {include_dir}")


def add_directory_to_zip(
    archive: zipfile.ZipFile,
    source_dir: Path,
    archive_root: str,
) -> None:
    files = list(source_dir.rglob("*"))

    if not files:
        archive.writestr(f"{archive_root}/", "")
        return

    for item in files:
        relative = item.relative_to(source_dir).as_posix()
        archive_name = f"{archive_root}/{relative}"

        if item.is_dir():
            archive.writestr(f"{archive_name}/", "")
        elif item.is_file():
            archive.write(item, archive_name)


def package_platform(
    platform_key: str,
    sources: list[tuple[str, Path]],
    version: str,
) -> Path:
    info = PLATFORMS[platform_key]

    PACKAGE_ROOT.mkdir(parents=True, exist_ok=True)

    package_name = (
        f"NeoWPF-{version}-{info['architecture']}-{info['package_platform']}.zip"
    )
    package_path = PACKAGE_ROOT / package_name

    with zipfile.ZipFile(
        package_path,
        mode="w",
        compression=zipfile.ZIP_DEFLATED,
        compresslevel=9,
    ) as archive:
        add_common_package_files(archive)

        for config, source in sources:
            archive.write(source, (Path(config) / source.name).as_posix())

    return package_path


def add_common_package_files(archive: zipfile.ZipFile) -> None:
    archive.write(LICENSE_FILE, "LICENSE")
    add_directory_to_zip(archive, INCLUDE_ROOT / "NeoWPF", "NeoWPF")
    add_directory_to_zip(archive, INCLUDE_ROOT / "NeoNaml", "NeoNaml")


def package_combined(
    sources: list[tuple[str, str, Path]],
    version: str,
) -> Path:
    PACKAGE_ROOT.mkdir(parents=True, exist_ok=True)
    package_path = PACKAGE_ROOT / f"NeoWPF-{version}-all-platforms.zip"

    with zipfile.ZipFile(
        package_path,
        mode="w",
        compression=zipfile.ZIP_DEFLATED,
        compresslevel=9,
    ) as archive:
        add_common_package_files(archive)

        for platform, config, source in sources:
            archive.write(
                source,
                (Path(platform) / config / source.name).as_posix(),
            )

    return package_path


def main() -> int:
    args = parse_args()

    try:
        version = read_version()
        validate_package_files()
    except (FileNotFoundError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    platforms = selected_platforms(args)
    configs = selected_configs(args)

    print(f"NeoWPF {version} pack")
    print(
        f"platforms: {', '.join(PLATFORMS[p]['file_platform'] for p in platforms)}"
    )
    print(f"configs:   {', '.join(configs)}")
    print()

    missing: list[tuple[str, str]] = []
    packed: list[Path] = []
    combined_sources: list[tuple[str, str, Path]] = []

    for platform_key in platforms:
        platform_name = PLATFORMS[platform_key]["file_platform"]
        sources: list[tuple[str, Path]] = []
        platform_missing = False

        for config in configs:
            source = find_library(platform_key, config, version)

            if source is None:
                missing.append((platform_name, config))
                platform_missing = True
                print(f"[missing] {platform_name} {config}")
                continue

            sources.append((config, source))
            if args.combined:
                combined_sources.append(
                    (PLATFORMS[platform_key]["folder"], config, source)
                )
            print(f"[found]   {source}")

        if args.combined:
            continue

        if platform_missing:
            print(f"[skipped] {platform_name} archive (missing requested build)")
            continue

        package = package_platform(platform_key, sources, version)
        packed.append(package)
        print(f"[packed]  {package}")
        print()

    if args.combined and combined_sources:
        package = package_combined(combined_sources, version)
        packed.append(package)
        print(f"[packed]  {package}")
        print()

    if missing:
        print("some requested builds were missing:")
        for platform_name, config in missing:
            print(f"  - {platform_name} {config}")

    if not packed:
        print("nothing got packed.", file=sys.stderr)
        return 1

    if missing:
        print("packing finished with missing builds.", file=sys.stderr)
        return 1

    print(f"done. packed {len(packed)} archive(s) into {PACKAGE_ROOT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
