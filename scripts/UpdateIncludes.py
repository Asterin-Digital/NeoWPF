import os
import re
from pathlib import Path

def scan_headers(include_dir):
    headers = {}
    duplicates = []
    
    for root, _, files in os.walk(include_dir):
        for f in files:
            if f.endswith(('.h', '.hpp')):
                full = Path(root) / f
                rel = full.relative_to(include_dir)
                rel_str = str(rel).replace('\\', '/')
                
                if f in headers:
                    duplicates.append(f)
                else:
                    headers[f] = rel_str
    
    return headers, duplicates

def fix_includes():
    src_dir = "src"
    include_dir = "include/NeoWPF"
    
    if not os.path.exists(src_dir):
        print(f"Error: {src_dir} not found")
        return
    
    if not os.path.exists(include_dir):
        print(f"Error: {include_dir} not found")
        return
    
    headers, duplicates = scan_headers(include_dir)
    
    if duplicates:
        print(f"Warning: Duplicate header names found - {', '.join(duplicates)}")
        print("These files will be ignored to prevent incorrect matches\n")
    
    all_changes = {}
    total_fixed = 0
    
    # Scan both src and include directories
    for scan_root in [src_dir, include_dir]:
        for root, _, files in os.walk(scan_root):
            for f in files:
                if not f.endswith(('.cpp', '.cc', '.cxx', '.h', '.hpp')):
                    continue
                    
                path = Path(root) / f
                with open(path, 'r') as file:
                    content = file.read()
                
                new_content = content
                changes = []
                
                for match in re.finditer(r'#include\s*<([^>]+)>', content):
                    old = match.group(1)
                    base = os.path.basename(old)
                    
                    if base in headers and base not in duplicates:
                        new_path = "NeoWPF/" + headers[base]
                        if old != new_path:
                            changes.append((old, new_path))
                            new_content = new_content.replace(f'<{old}>', f'<{new_path}>')
                
                if changes:
                    all_changes[path] = (new_content, changes)
    
    if not all_changes:
        print("No changes needed")
        return
    
    for path, (new_content, changes) in all_changes.items():
        print(f"\n{path}:")
        for old, new in changes:
            print(f"  <{old}> -> <{new}>")
    
    response = input(f"\nUpdate all {len(all_changes)} files? (y/n): ").strip().lower()
    if response == 'y':
        for path, (new_content, changes) in all_changes.items():
            with open(path, 'w') as file:
                file.write(new_content)
            total_fixed += len(changes)
        print(f"\nFixed {total_fixed} includes in {len(all_changes)} files")
    else:
        print("Cancelled")

if __name__ == '__main__':
    fix_includes()