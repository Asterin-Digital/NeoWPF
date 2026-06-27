################################################
## This is AI-Generated content
## Not made by a human!
## NeoWPF — A C++ UI Framework similar to WPF :3
################################################

import os
import re
import time
from pathlib import Path
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox


try:
    from watchdog.observers import Observer
    from watchdog.events import FileSystemEventHandler
    WATCHDOG_AVAILABLE = True
except ImportError:
    WATCHDOG_AVAILABLE = False


SRC_DIR = "src"
INCLUDE_DIR = "include/NeoWPF"
WATCHED_DIRS = [SRC_DIR, "include"]
DEBOUNCE_DELAY = 1000


def scan_headers(include_dir):
    headers = {}
    duplicates = set()
    include_path = Path(include_dir)
    if not include_path.is_dir():
        return headers, duplicates

    for root, _, files in os.walk(include_dir):
        for f in files:
            if f.endswith(('.h', '.hpp')):
                full = Path(root) / f
                try:
                    rel = full.relative_to(include_dir).as_posix()
                except ValueError:
                    continue
                if f in headers:
                    duplicates.add(f)
                else:
                    headers[f] = rel
    for dup in duplicates:
        headers.pop(dup, None)
    return headers, duplicates

def fix_includes_content(content, headers, duplicates):
    new_content = content
    changes = []
    for match in re.finditer(r'#include\s*<([^>]+)>', content):
        old_full = match.group(1)
        base = os.path.basename(old_full)
        if base in headers and base not in duplicates:
            correct_rel = headers[base]
            correct_full = f"NeoWPF/{correct_rel}"
            if old_full != correct_full:
                changes.append((old_full, correct_full))
                new_content = new_content.replace(
                    f'<{old_full}>', f'<{correct_full}>'
                )
    return new_content, changes


class ConfirmationPanel(tk.Frame):
    """In‑app panel to show changes and ask Yes/No."""
    def __init__(self, parent, changes_per_file, on_yes, on_no):
        super().__init__(parent, bg='#2e2e2e', highlightbackground='#555',
                         highlightthickness=1)
        self.on_yes = on_yes
        self.on_no = on_no


        summary = tk.Text(self, wrap=tk.WORD, bg='#1e1e1e', fg='#dcdcdc',
                          font=('Consolas', 10), height=10, relief=tk.FLAT)
        summary.pack(fill=tk.BOTH, expand=True, padx=5, pady=(5,0))

        lines = []
        total = 0
        for path, changes in changes_per_file.items():
            lines.append(str(path))
            for old, new in changes:
                lines.append(f"  <{old}>  →  <{new}>")
                total += 1
        summary.insert(tk.END, "\n".join(lines))
        summary.configure(state='disabled')

        btn_frame = tk.Frame(self, bg='#2e2e2e')
        btn_frame.pack(fill=tk.X, pady=5, padx=5)

        yes_btn = tk.Button(btn_frame, text="Yes – Apply changes",
                            command=self._on_yes, bg='#4a4a4a', fg='white',
                            activebackground='#5a5a5a')
        yes_btn.pack(side=tk.LEFT, padx=5)

        no_btn = tk.Button(btn_frame, text="No – Skip",
                           command=self._on_no, bg='#4a4a4a', fg='white',
                           activebackground='#5a5a5a')
        no_btn.pack(side=tk.LEFT, padx=5)

        total_label = tk.Label(btn_frame, text=f"{total} include(s) to update",
                               bg='#2e2e2e', fg='#aaa')
        total_label.pack(side=tk.RIGHT, padx=10)

    def _on_yes(self):
        self.destroy()
        if self.on_yes:
            self.on_yes()

    def _on_no(self):
        self.destroy()
        if self.on_no:
            self.on_no()


class IncludeFixerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Include Fixer Watcher")
        self.root.geometry("800x600")

        self.running = False
        self.watcher = None
        self.debounce_id = None
        self.confirm_panel = None
        self.pending_scan = False


        self.style = ttk.Style()
        self.style.theme_use('clam')
        bg = '#2e2e2e'
        fg = '#dcdcdc'
        self.root.configure(bg=bg)
        self.style.configure('.', background=bg, foreground=fg,
                             fieldbackground=bg, troughcolor=bg,
                             selectbackground='#3e3e3e')
        self.style.map('TButton',
                       background=[('active', '#4a4a4a'), ('pressed', '#5a5a5a')])
        self.style.configure('TLabel', background=bg, foreground=fg)
        self.style.configure('TCheckbutton', background=bg, foreground=fg)
        self.style.configure('TFrame', background=bg)


        self.main_frame = ttk.Frame(root, padding=10)
        self.main_frame.pack(fill=tk.BOTH, expand=True)


        control_frame = ttk.Frame(self.main_frame)
        control_frame.pack(fill=tk.X, pady=(0, 10))

        self.start_stop_btn = ttk.Button(control_frame, text="Start Monitoring",
                                         command=self.toggle_monitoring)
        self.start_stop_btn.pack(side=tk.LEFT, padx=5)

        self.status_var = tk.StringVar(value="Idle")
        status_label = ttk.Label(control_frame, textvariable=self.status_var)
        status_label.pack(side=tk.RIGHT, padx=10)


        self.log = scrolledtext.ScrolledText(self.main_frame, wrap=tk.WORD,
                                             bg='#1e1e1e', fg='#dcdcdc',
                                             insertbackground='white',
                                             font=('Consolas', 10))
        self.log.pack(fill=tk.BOTH, expand=True)

        self._periodic_check()

    def log_message(self, msg):
        timestamp = time.strftime("%H:%M:%S")
        self.log.insert(tk.END, f"[{timestamp}] {msg}\n")
        self.log.see(tk.END)

    def toggle_monitoring(self):
        if self.running:
            self.stop_monitoring()
        else:
            self.start_monitoring()

    def start_monitoring(self):
        if not WATCHDOG_AVAILABLE:
            messagebox.showerror("Missing dependency",
                                 "watchdog is required but not installed.\n"
                                 "Install it with: pip install watchdog")
            return
        if not os.path.isdir(SRC_DIR):
            messagebox.showerror("Error", f"Source directory '{SRC_DIR}' not found.")
            return
        if not os.path.isdir(INCLUDE_DIR):
            messagebox.showerror("Error", f"Include directory '{INCLUDE_DIR}' not found.")
            return

        self.running = True
        self.start_stop_btn.configure(text="Stop Monitoring")
        self.status_var.set("Initial scan...")
        self.log_message("=== Monitoring started ===")
        self.do_full_fix()
        self._start_watchdog()

    def stop_monitoring(self):
        self.running = False
        self.start_stop_btn.configure(text="Start Monitoring")
        self.status_var.set("Stopped")
        self.log_message("=== Monitoring stopped ===")
        self._stop_watcher()
        if self.debounce_id:
            self.root.after_cancel(self.debounce_id)
            self.debounce_id = None
        self._dismiss_confirmation()




    def do_full_fix(self):
        if not self.running:
            return

        if self.confirm_panel is not None:
            self.pending_scan = True
            return

        self.status_var.set("Scanning headers...")
        headers, duplicates = scan_headers(INCLUDE_DIR)
        if duplicates:
            self.log_message(f"Warning: ambiguous header names ignored: {', '.join(sorted(duplicates))}")

        changes_per_file = {}
        total_changes = 0

        for scan_root in [SRC_DIR, "include"]:
            if not os.path.isdir(scan_root):
                continue
            for root, _, files in os.walk(scan_root):
                for f in files:
                    if not f.endswith(('.cpp', '.cc', '.cxx', '.h', '.hpp')):
                        continue
                    path = Path(root) / f
                    try:
                        with open(path, 'r', encoding='utf-8') as fh:
                            content = fh.read()
                    except Exception as e:
                        self.log_message(f"Error reading {path}: {e}")
                        continue
                    new_content, changes = fix_includes_content(content, headers, duplicates)
                    if not changes:
                        continue
                    changes_per_file[path] = changes
                    total_changes += len(changes)

        if total_changes == 0:
            self.log_message("All includes are up to date.")
            self.status_var.set("Monitoring...")
            return


        for path, changes in changes_per_file.items():
            for old, new in changes:
                self.log_message(f"{path}: <{old}> → <{new}>")


        self._show_confirmation(changes_per_file)

    def _show_confirmation(self, changes_per_file):
        self.confirm_panel = ConfirmationPanel(
            self.main_frame,
            changes_per_file,
            on_yes=lambda: self._apply_changes(changes_per_file),
            on_no=self._skip_changes
        )
        self.confirm_panel.pack(fill=tk.BOTH, padx=5, pady=5,
                                before=self.log)

    def _dismiss_confirmation(self):
        if self.confirm_panel:
            self.confirm_panel.destroy()
            self.confirm_panel = None

    def _apply_changes(self, changes_per_file):
        """Called when user clicks Yes."""
        try:
            for path, changes in changes_per_file.items():
                try:
                    with open(path, 'r', encoding='utf-8') as fh:
                        content = fh.read()
                    new_content = content
                    for old, new in changes:
                        new_content = new_content.replace(f'<{old}>', f'<{new}>')
                    with open(path, 'w', encoding='utf-8') as fh:
                        fh.write(new_content)
                    self.log_message(f"  Applied changes to {path}")
                except Exception as e:
                    self.log_message(f"Failed to write {path}: {e}")
            total = sum(len(c) for c in changes_per_file.values())
            self.log_message(f"Applied {total} fix(es).")
        finally:
            self._after_confirmation()

    def _skip_changes(self):
        """Called when user clicks No."""
        self.log_message("Changes skipped by user.")
        self._after_confirmation()

    def _after_confirmation(self):
        self.confirm_panel = None
        if self.running:
            self.status_var.set("Monitoring...")
            if self.pending_scan:
                self.pending_scan = False
                self.do_full_fix()
        else:
            self.status_var.set("Stopped")

    def schedule_fix(self):
        if not self.running:
            return
        if self.debounce_id:
            self.root.after_cancel(self.debounce_id)
        self.debounce_id = self.root.after(DEBOUNCE_DELAY, self._on_debounce_fix)

    def _on_debounce_fix(self):
        self.debounce_id = None
        self.do_full_fix()

    def _periodic_check(self):
        self.root.after(200, self._periodic_check)




    def _start_watchdog(self):
        class Handler(FileSystemEventHandler):
            def __init__(self, app):
                self.app = app
            def on_any_event(self, event):
                if event.is_directory:
                    return
                self.app.schedule_fix()

        observer = Observer()
        for d in WATCHED_DIRS:
            if os.path.isdir(d):
                observer.schedule(Handler(self), d, recursive=True)
        observer.start()
        self.watcher = observer

    def _stop_watcher(self):
        if self.watcher:
            self.watcher.stop()
            self.watcher = None


def main():
    root = tk.Tk()
    app = IncludeFixerApp(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.stop_monitoring(), root.destroy()))
    root.mainloop()

if __name__ == "__main__":
    main()