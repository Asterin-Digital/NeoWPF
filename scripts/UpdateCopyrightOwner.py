from pathlib import Path
import argparse

OLD_OWNER = "Copyright (c) 2026 NeoWPF"
NEW_OWNER = "Copyright (c) 2026 Asterin Digital"

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


def update_owner(path: Path, dry_run: bool = False):
    original_text = path.read_text(encoding="utf-8")
    if OLD_OWNER not in original_text:
        return False

    updated_text = original_text.replace(OLD_OWNER, NEW_OWNER)
    if not dry_run:
        path.write_text(updated_text, encoding="utf-8")

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Update the copyright owner in NeoWPF .h files."
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

    for header_file in sorted(iter_header_files(root)):
        if update_owner(header_file, dry_run=args.dry_run):
            updated.append(str(header_file.relative_to(root)))

    action = "Would update" if args.dry_run else "Updated"
    print(f"{action}: {len(updated)} .h file(s)")

    if updated:
        print("\nModified files:")
        for path in updated:
            print(f"- {path}")


if __name__ == "__main__":
    main()
