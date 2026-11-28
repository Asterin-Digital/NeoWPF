from pathlib import Path

HEADER = """/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/"""

LICENSE_MARKER = "See LICENSE file in the project root"

EXCLUDED_DIRS = {
    ".git", ".vs", ".vscode", "build", "Debug",
    "out", "private", "reports", "vcpkg_installed", "include",
}


def iter_header_files(root: Path):
    for path in root.rglob("*.h"):
        relative = path.relative_to(root)

        if not path.is_file():
            continue
        if path.name == "NeoWPF.h":
            continue
        if any(part in EXCLUDED_DIRS for part in relative.parts[:-1]):
            continue

        yield path


def add_header(path: Path):
    text = path.read_text(encoding="utf-8-sig")

    if HEADER in text:
        return False

    marker_pos = text.find(LICENSE_MARKER)
    if marker_pos == -1:
        print(f"Skipped (license marker not found): {path}")
        return False

    comment_end = text.find("*/", marker_pos)
    if comment_end == -1:
        print(f"Skipped (license comment not closed): {path}")
        return False

    comment_end += 2

    updated = (
        text[:comment_end]
        + "\n\n"
        + HEADER
        + "\n\n"
        + text[comment_end:].lstrip("\n")
    )

    path.write_text(updated, encoding="utf-8")
    return True


def main():
    include_dir = Path.cwd().parent / "include"

    if not include_dir.is_dir():
        raise FileNotFoundError(include_dir)

    updated = 0
    skipped = 0

    for path in sorted(iter_header_files(include_dir)):
        if add_header(path):
            updated += 1
            print(f"Updated: {path.relative_to(include_dir)}")
        else:
            skipped += 1

    print(f"\nUpdated: {updated}")
    print(f"Skipped: {skipped}")


if __name__ == "__main__":
    main()