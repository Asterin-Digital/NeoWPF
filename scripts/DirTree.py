import os
import sys

IGNORED = {".git", "__pycache__", ".DS_Store", "Library", "Temp", "obj", "Build",
           "Logs", "UserSettings", "*.meta", "*.cs.meta", "*.unity.meta", "x64", ".vs", "out", "cmake"}

def should_ignore(name):
    if name.endswith(".meta"):
        return True
    if name in IGNORED:
        return True
    return False

def count_lines_in_file(filepath):
    """Count total lines and non-empty lines in a file."""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            total_lines = len(lines)
            non_empty_lines = sum(1 for line in lines if line.strip())
            return total_lines, non_empty_lines
    except:
        return 0, 0

def count_lines_in_directory(root_dir):
    """Count all lines of code in the directory recursively."""
    total_lines = 0
    non_empty_lines = 0

    for dirpath, dirnames, filenames in os.walk(root_dir):

        dirnames[:] = [d for d in dirnames if not should_ignore(d)]

        for filename in filenames:
            if should_ignore(filename):
                continue


            code_extensions = {'.py', '.cs', '.java', '.cpp', '.c', '.h', '.hpp',
                             '.js', '.ts', '.html', '.css', '.txt', '.md', '.json',
                             '.xml', '.yaml', '.yml', '.sh', '.bat', '.ps1', ".dat"}

            if any(filename.endswith(ext) for ext in code_extensions):
                filepath = os.path.join(dirpath, filename)
                total, non_empty = count_lines_in_file(filepath)
                total_lines += total
                non_empty_lines += non_empty

    return total_lines, non_empty_lines

def build_tree(root_dir, prefix=""):
    if not os.path.isdir(root_dir):
        return

    items = [item for item in sorted(os.listdir(root_dir)) if not should_ignore(item)]

    for index, item in enumerate(items):
        path = os.path.join(root_dir, item)
        is_last = index == len(items) - 1

        if is_last:
            print(prefix + "└── " + item)
            new_prefix = prefix + "    "
        else:
            print(prefix + "├── " + item)
            new_prefix = prefix + "│   "

        if os.path.isdir(path):
            build_tree(path, new_prefix)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <root_dir>")
        sys.exit(1)

    root = sys.argv[1]
    if not os.path.exists(root):
        print(f"Error: {root} does not exist")
        sys.exit(1)


    print(f"Scanning: {root}")
    print("-" * 60)
    total, non_empty = count_lines_in_directory(root)
    print(f"   Code Statistics:")
    print(f"   Total lines (including whitespace): {total:,}")
    print(f"   Lines of code (excluding whitespace): {non_empty:,}")
    print("-" * 60)
    print()


    print(root)
    build_tree(root)