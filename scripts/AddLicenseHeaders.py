from pathlib import Path
import argparse

HEADER = """/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

"""

EXCLUDED_DIRS = {
    ".git",
    ".vs",
    ".vscode",
    "build",
    "Debug",
    "out",
    "private",
    "reports",
    "vcpkg_installed",
}


def iter_header_files(root: Path):
    for path in root.rglob("*.h"):
        if not path.is_file():
            continue

        rel_parts = path.relative_to(root).parts
        if any(part in EXCLUDED_DIRS for part in rel_parts[:-1]):
            continue

        yield path


def add_header(path: Path, dry_run: bool = False):
    original_text = path.read_text(encoding="utf-8")
    normalized = original_text.lstrip("\ufeff")

    if normalized.startswith(HEADER):
        return False

    new_text = HEADER + normalized

    if not dry_run:
        path.write_text(new_text, encoding="utf-8")

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Prepend the NeoWPF MIT license header to every .h file in the project."
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="Project root to scan. Defaults to the NeoWPF repository root.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show which files would be updated without changing them.",
    )
    args = parser.parse_args()

    root = args.root.resolve()
    updated = []
    skipped = 0

    for header_file in sorted(iter_header_files(root)):
        changed = add_header(header_file, dry_run=args.dry_run)
        if changed:
            updated.append(str(header_file.relative_to(root)))
        else:
            skipped += 1

    action = "Would update" if args.dry_run else "Updated"
    print(f"{action}: {len(updated)} .h file(s)")

    if updated:
        print("\nModified files:")
        for path in updated:
            print(f"- {path}")

    print(f"Skipped already-updated files: {skipped}")


if __name__ == "__main__":
    main()
