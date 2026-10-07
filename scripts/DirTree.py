import os
import sys

IGNORED_NAMES = {
    ".git", "__pycache__", ".DS_Store", "Library", "Temp", "obj", "Build",
    "Logs", "UserSettings", "x64", ".vs", "out", "cmake", "vcpkg_installed", "build", "bin", ".vscode", "Testing", "Debug", "NWpfBuild", "tools", "reports", "NLintBuild", ".github"
}

IGNORED_PATHS = {
    "NeoWPF", "private/WPF"
}

IGNORED_EXTENSIONS = {".meta"}


def normalize_relpath(relpath):
    return relpath.replace("\\", "/").strip("/")


def make_ignore_checker(root_dir):
    ignored_paths_norm = {normalize_relpath(p) for p in IGNORED_PATHS}

    def is_ignored(relpath):
        relpath = normalize_relpath(relpath)
        name = relpath.rsplit("/", 1)[-1]

        if name in IGNORED_NAMES:
            return True

        for ext in IGNORED_EXTENSIONS:
            if name.endswith(ext):
                return True

        if relpath in ignored_paths_norm:
            return True

        return False

    return is_ignored


def count_lines_in_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            total_lines = len(lines)
            non_empty_lines = sum(1 for line in lines if line.strip())
            return total_lines, non_empty_lines
    except Exception:
        return 0, 0


def count_lines_in_directory(root_dir, is_ignored):
    total_lines = 0
    non_empty_lines = 0

    code_extensions = {'.py', '.cs', '.java', '.cpp', '.c', '.h', '.hpp',
                       '.js', '.ts', '.html', '.css', '.txt', '.md', '.json',
                       '.xml', '.yaml', '.yml', '.sh', '.bat', '.ps1', '.dat'}

    for dirpath, dirnames, filenames in os.walk(root_dir):
        rel_dir = os.path.relpath(dirpath, root_dir)
        if rel_dir == ".":
            rel_dir = ""

        kept = []
        for d in dirnames:
            child_rel = normalize_relpath(os.path.join(rel_dir, d))
            if is_ignored(child_rel):
                continue
            kept.append(d)
        dirnames[:] = kept

        for filename in filenames:
            child_rel = normalize_relpath(os.path.join(rel_dir, filename))
            if is_ignored(child_rel):
                continue

            if any(filename.endswith(ext) for ext in code_extensions):
                filepath = os.path.join(dirpath, filename)
                total, non_empty = count_lines_in_file(filepath)
                total_lines += total
                non_empty_lines += non_empty

    return total_lines, non_empty_lines


def build_tree(root_dir, is_ignored, rel_dir="", prefix=""):
    abs_dir = os.path.join(root_dir, rel_dir) if rel_dir else root_dir
    if not os.path.isdir(abs_dir):
        return

    items = []
    for item in sorted(os.listdir(abs_dir)):
        child_rel = normalize_relpath(os.path.join(rel_dir, item))
        if is_ignored(child_rel):
            continue
        items.append(item)

    for index, item in enumerate(items):
        path = os.path.join(abs_dir, item)
        child_rel = normalize_relpath(os.path.join(rel_dir, item))
        is_last = index == len(items) - 1

        if is_last:
            print(prefix + "\u2514\u2500\u2500 " + item)
            new_prefix = prefix + "    "
        else:
            print(prefix + "\u251c\u2500\u2500 " + item)
            new_prefix = prefix + "\u2502   "

        if os.path.isdir(path):
            build_tree(root_dir, is_ignored, child_rel, new_prefix)


if __name__ == "__main__":
    if sys.stdout.isatty():
        sys.stdout.reconfigure(encoding="utf-8")
    else:
        sys.stdout.reconfigure(encoding="utf-8-sig")

    if len(sys.argv) != 2:
        print("Usage: python script.py <root_dir>")
        sys.exit(1)

    root = sys.argv[1]
    if not os.path.exists(root):
        print(f"Error: {root} does not exist")
        sys.exit(1)

    is_ignored = make_ignore_checker(root)

    print(f"Scanning: {root}")
    print("-" * 60)
    total, non_empty = count_lines_in_directory(root, is_ignored)
    print("   Code Statistics:")
    print(f"   Total lines (including whitespace): {total:,}")
    print(f"   Lines of code (excluding whitespace): {non_empty:,}")
    print("-" * 60)
    print()

    print(root)
    build_tree(root, is_ignored)

# python "C:\Users\Katsu\source\repos\NeoWPF\scripts\DirTree.py" "C:\Users\Katsu\source\repos\NeoWPF"