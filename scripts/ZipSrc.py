import os
from pathlib import Path
import zipfile


ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = ROOT / "private" / "Zipped"
OUTPUT_FILE = OUTPUT_DIR / "NeoWPF_Source.zip"
BUILD_OUTPUT_DIRECTORY_NAMES = frozenset(
    {"artifacts", "bin", "build", "builds", "obj", "out"}
)

SOURCE_DIRECTORIES = (
    ROOT / "src",
    ROOT / "include",
    ROOT / "tests",
    ROOT / "private",
    ROOT / "docs",
    ROOT / "cmake",
    ROOT / "scripts",
    ROOT / "NeoCM" / "src",
    ROOT / "NeoCM" / "include",
    ROOT / "NeoPM" / "src",
    ROOT / "NeoPM" / "include",
    ROOT / "NeoLint" / "src",
    ROOT / "NeoLint" / "include",
    ROOT / "dotnet",
)


def is_excluded(path: Path) -> bool:
    if path == OUTPUT_DIR or OUTPUT_DIR in path.parents:
        return True

    return any(
        part.casefold() in BUILD_OUTPUT_DIRECTORY_NAMES
        for part in path.relative_to(ROOT).parts
    )


def iter_dir_entries(source_dir: Path):
    for current_dir, directories, files in os.walk(source_dir):
        current_path = Path(current_dir)
        directories[:] = sorted(
            name
            for name in directories
            if not is_excluded(current_path / name)
        )

        for name in directories:
            yield current_path / name

        for name in sorted(files):
            entry = current_path / name
            if not is_excluded(entry):
                yield entry


def add_directory_to_zip(archive: zipfile.ZipFile, source_dir: Path) -> None:
    entries = list(iter_dir_entries(source_dir))

    if not entries:
        archive.writestr(f"{source_dir.relative_to(ROOT).as_posix()}/", "")
        return

    for entry in entries:
        archive_name = entry.relative_to(ROOT).as_posix()
        if entry.is_dir():
            archive.writestr(f"{archive_name}/", "")
        elif entry.is_file():
            archive.write(entry, archive_name)


def main() -> None:
    for source_dir in SOURCE_DIRECTORIES:
        if not source_dir.is_dir():
            raise FileNotFoundError(f"Source dir not found: {source_dir}")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(
        OUTPUT_FILE,
        mode="w",
        compression=zipfile.ZIP_DEFLATED,
        compresslevel=9,
    ) as archive:
        for source_dir in SOURCE_DIRECTORIES:
            add_directory_to_zip(archive, source_dir)

    print(f"Created source archive: {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
