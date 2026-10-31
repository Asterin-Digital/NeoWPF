import os
import sys
import subprocess
import tempfile

IGNORED_NAMES = {
    ".git", "__pycache__", ".DS_Store", "Library", "Temp", "obj", "Build",
    "Logs", "UserSettings", "x64", ".vs", "out", "cmake", "vcpkg_installed",
    "build", "bin", ".vscode", "Testing", "Debug", "NWpfBuild", "tools",
    "reports", "NLintBuild", ".github"
}

IGNORED_PATHS = {
    "NeoWPF", "private/WPF"
}

IGNORED_EXTENSIONS = {".meta"}


def normalize_relpath(relpath):
    return relpath.replace("\\", "/").strip("/")


def make_ignore_checker():
    ignored_paths_norm = {normalize_relpath(p) for p in IGNORED_PATHS}

    def is_ignored(relpath):
        relpath = normalize_relpath(relpath)
        name = relpath.rsplit("/", 1)[-1]

        if name in IGNORED_NAMES:
            return True

        if any(name.endswith(ext) for ext in IGNORED_EXTENSIONS):
            return True

        return relpath in ignored_paths_norm

    return is_ignored


def get_commit_files(commit):
    result = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", "-z", commit],
        capture_output=True,
        check=True
    )

    return [
        path.decode("utf-8", errors="replace")
        for path in result.stdout.split(b"\0")
        if path
    ]


def build_tree(files, is_ignored):
    tree = {}

    for filepath in files:
        parts = normalize_relpath(filepath).split("/")

        current = tree
        ignored = False

        for part in parts:
            path = "/".join(parts[:parts.index(part) + 1])

            if is_ignored(path):
                ignored = True
                break

            current = current.setdefault(part, {})

        if not ignored:
            current["__file__"] = True

    def print_tree(node, prefix=""):
        items = sorted(
            key for key in node
            if key != "__file__"
        )

        for index, item in enumerate(items):
            is_last = index == len(items) - 1
            connector = "└── " if is_last else "├── "

            print(prefix + connector + item)

            if isinstance(node[item], dict):
                new_prefix = prefix + ("    " if is_last else "│   ")
                print_tree(node[item], new_prefix)

    print_tree(tree)


def main():
    if sys.stdout.isatty():
        sys.stdout.reconfigure(encoding="utf-8")
    else:
        sys.stdout.reconfigure(encoding="utf-8-sig")

    if len(sys.argv) != 2:
        print("Usage: python script.py <commit_hash>")
        sys.exit(1)

    commit = sys.argv[1]

    try:
        subprocess.run(
            ["git", "rev-parse", "--verify", f"{commit}^{{commit}}"],
            capture_output=True,
            check=True
        )
    except subprocess.CalledProcessError:
        print(f"Error: Invalid commit: {commit}")
        sys.exit(1)

    is_ignored = make_ignore_checker()

    try:
        files = get_commit_files(commit)
    except subprocess.CalledProcessError:
        print(f"Error: Could not read commit: {commit}")
        sys.exit(1)

    print(f"Commit: {commit}")
    print("-" * 60)
    print()
    print(os.path.basename(os.path.abspath(os.getcwd())))
    build_tree(files, is_ignored)


if __name__ == "__main__":
    main()