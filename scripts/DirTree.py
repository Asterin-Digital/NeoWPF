import os
import sys

IGNORED = {".git", "__pycache__", ".DS_Store", "Library", "Temp", "obj", "Build", 
           "Logs", "UserSettings", "*.meta", "*.cs.meta", "*.unity.meta", "x64", ".vs"}

def should_ignore(name):
    if name.endswith(".meta"):
        return True
    if name in IGNORED:
        return True
    return False

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
    
    print(root)
    build_tree(root)