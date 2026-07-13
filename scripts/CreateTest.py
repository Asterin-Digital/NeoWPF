import json
import os
import re
from datetime import datetime
from pathlib import Path
import tkinter as tk
from tkinter import messagebox, simpledialog, ttk

ROOT_DIR = Path(__file__).resolve().parent.parent
TESTS_DIR = ROOT_DIR / "tests"
METADATA_FILE = TESTS_DIR / ".create_test_metadata.json"
METADATA_START = "// TEST_METADATA_START"
METADATA_END = "// TEST_METADATA_END"


def ensure_tests_dir() -> None:
    TESTS_DIR.mkdir(parents=True, exist_ok=True)


def load_metadata() -> dict:
    if not METADATA_FILE.exists():
        return {}
    try:
        with METADATA_FILE.open("r", encoding="utf-8") as file:
            return json.load(file)
    except (json.JSONDecodeError, OSError):
        return {}


def save_metadata(metadata: dict) -> None:
    try:
        with METADATA_FILE.open("w", encoding="utf-8") as file:
            json.dump(metadata, file, indent=2, ensure_ascii=False)
    except OSError as exc:
        messagebox.showerror("Save Error", f"Failed to save metadata: {exc}")


def sanitize_filename(name: str) -> str:
    name = name.strip()
    name = re.sub(r"[\\/:*?\"<>|]+", "", name)
    name = re.sub(r"\s+", "_", name)
    return name or "NewTest"


def get_subfolders() -> list[str]:
    ensure_tests_dir()
    return sorted(
        [item.name for item in TESTS_DIR.iterdir() if item.is_dir() and not item.name.startswith('.')]
    )


def build_metadata_block(metadata: dict) -> str:
    lines = [METADATA_START]
    for key, value in metadata.items():
        lines.append(f"// {key}: {value}")
    lines.append(METADATA_END)
    return "\n".join(lines) + "\n"


def build_initial_cpp(test_name: str, description: str, relative_path: str) -> str:
    header = (
        f"// {test_name}.cpp\n"
        f"// Description: {description.strip()}\n"
        "#include <iostream>\n\n"
    )
    body = (
        "int main()\n"
        "{\n"
        "    std::cout << \"Test running: " + test_name.replace('"', '\\"') + "\" << std::endl;\n"
        "    return 0;\n"
        "}\n\n"
    )
    metadata = {
        "Test Name": test_name,
        "Description": description.strip(),
        "Created": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "Result": "Not run",
        "Notes": "",
        "Path": relative_path.replace('\\', '/')
    }
    return header + body + build_metadata_block(metadata), metadata


def append_or_replace_metadata_block(source: str, metadata: dict) -> str:
    new_block = build_metadata_block(metadata)
    start = source.find(METADATA_START)
    end = source.find(METADATA_END, start + len(METADATA_START))
    if start != -1 and end != -1:
        end += len(METADATA_END)
        before = source[:start].rstrip() + "\n\n"
        after = source[end:].lstrip("\n")
        return before + new_block + after
    if not source.endswith("\n"):
        source += "\n"
    return source + "\n" + new_block


def read_test_file(file_path: Path) -> str:
    with file_path.open("r", encoding="utf-8", errors="replace") as file:
        return file.read()


def write_test_file(file_path: Path, content: str) -> None:
    with file_path.open("w", encoding="utf-8") as file:
        file.write(content)


class TestCreatorApp(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Create or Update Test")
        self.geometry("720x520")
        self.resizable(False, False)

        self.metadata = load_metadata()
        self.subfolder_names = get_subfolders()

        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill="both", expand=True, padx=10, pady=10)

        self.create_frame = ttk.Frame(self.notebook)
        self.update_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.create_frame, text="Create Test")
        self.notebook.add(self.update_frame, text="Update Result")

        self.build_create_tab()
        self.build_update_tab()

    def build_create_tab(self) -> None:
        padding = {"padx": 8, "pady": 6}

        ttk.Label(self.create_frame, text="Test Name:").grid(row=0, column=0, sticky="w", **padding)
        self.test_name_entry = ttk.Entry(self.create_frame, width=50)
        self.test_name_entry.grid(row=0, column=1, sticky="ew", **padding)

        ttk.Label(self.create_frame, text="What it tests:").grid(row=1, column=0, sticky="nw", **padding)
        self.description_text = tk.Text(self.create_frame, width=56, height=10, wrap="word")
        self.description_text.grid(row=1, column=1, sticky="ew", **padding)

        ttk.Label(self.create_frame, text="Subfolder:").grid(row=2, column=0, sticky="w", **padding)
        self.subfolder_combo = ttk.Combobox(self.create_frame, values=self.subfolder_names + ["<Create new folder>"], state="readonly", width=48)
        self.subfolder_combo.grid(row=2, column=1, sticky="w", **padding)
        self.subfolder_combo.set(self.subfolder_names[0] if self.subfolder_names else "<Create new folder>")
        self.subfolder_combo.bind("<<ComboboxSelected>>", self.on_subfolder_changed)

        self.new_folder_label = ttk.Label(self.create_frame, text="New folder name:")
        self.new_folder_entry = ttk.Entry(self.create_frame, width=50)

        self.create_button = ttk.Button(self.create_frame, text="Create Test File", command=self.on_create_test)
        self.create_button.grid(row=5, column=1, sticky="e", **padding)

        self.create_frame.columnconfigure(1, weight=1)

        if self.subfolder_combo.get() == "<Create new folder>":
            self.show_new_folder_fields()

    def build_update_tab(self) -> None:
        padding = {"padx": 8, "pady": 6}

        ttk.Label(self.update_frame, text="Existing test file:").grid(row=0, column=0, sticky="w", **padding)
        self.test_list_combo = ttk.Combobox(self.update_frame, values=self.get_test_list(), state="readonly", width=70)
        self.test_list_combo.grid(row=0, column=1, sticky="ew", **padding)
        self.test_list_combo.bind("<<ComboboxSelected>>", self.on_test_selected)

        ttk.Label(self.update_frame, text="Result:").grid(row=1, column=0, sticky="w", **padding)
        self.result_entry = ttk.Entry(self.update_frame, width=50)
        self.result_entry.grid(row=1, column=1, sticky="ew", **padding)

        ttk.Label(self.update_frame, text="Notes:").grid(row=2, column=0, sticky="nw", **padding)
        self.notes_text = tk.Text(self.update_frame, width=56, height=10, wrap="word")
        self.notes_text.grid(row=2, column=1, sticky="ew", **padding)

        self.update_button = ttk.Button(self.update_frame, text="Save Result", command=self.on_update_result)
        self.update_button.grid(row=3, column=1, sticky="e", **padding)

        self.update_frame.columnconfigure(1, weight=1)

    def show_new_folder_fields(self) -> None:
        self.new_folder_label.grid(row=3, column=0, sticky="w", padx=8, pady=2)
        self.new_folder_entry.grid(row=3, column=1, sticky="ew", padx=8, pady=2)

    def hide_new_folder_fields(self) -> None:
        self.new_folder_label.grid_remove()
        self.new_folder_entry.grid_remove()

    def on_subfolder_changed(self, event: tk.Event) -> None:
        if self.subfolder_combo.get() == "<Create new folder>":
            self.show_new_folder_fields()
        else:
            self.hide_new_folder_fields()

    def get_test_list(self) -> list[str]:
        tests = sorted(self.metadata.keys())
        if not tests:
            tests = self.discover_test_files()
        return tests

    def discover_test_files(self) -> list[str]:
        results = []
        for root, _, files in os.walk(TESTS_DIR):
            for file_name in files:
                if file_name.lower().endswith(".cpp"):
                    results.append(str(Path(root, file_name).relative_to(TESTS_DIR).as_posix()))
        return sorted(results)

    def on_create_test(self) -> None:
        test_name = self.test_name_entry.get().strip()
        description = self.description_text.get("1.0", "end").strip()
        folder_choice = self.subfolder_combo.get()

        if not test_name:
            messagebox.showwarning("Missing Name", "Please enter a test name.")
            return
        if not description:
            messagebox.showwarning("Missing Description", "Please describe what the test covers.")
            return

        if folder_choice == "<Create new folder>":
            folder_name = self.new_folder_entry.get().strip()
            if not folder_name:
                messagebox.showwarning("Missing Folder", "Please enter a new folder name.")
                return
            folder_name = sanitize_filename(folder_name)
        else:
            folder_name = folder_choice

        sanitized_name = sanitize_filename(test_name)
        target_folder = TESTS_DIR / folder_name
        target_folder.mkdir(parents=True, exist_ok=True)
        relative_path = Path(folder_name, f"{sanitized_name}.cpp")
        target_path = target_folder / f"{sanitized_name}.cpp"

        if target_path.exists():
            if not messagebox.askyesno("Overwrite Test?", f"Test file already exists:\n{relative_path}\nOverwrite it?"):
                return

        content, file_metadata = build_initial_cpp(test_name, description, str(relative_path))
        write_test_file(target_path, content)

        metadata_key = str(relative_path.as_posix())
        self.metadata[metadata_key] = file_metadata
        save_metadata(self.metadata)

        if folder_name not in self.subfolder_names:
            self.subfolder_names.append(folder_name)
            self.subfolder_combo.config(values=self.subfolder_names + ["<Create new folder>"])

        self.test_list_combo.config(values=self.get_test_list())
        self.test_name_entry.delete(0, "end")
        self.description_text.delete("1.0", "end")
        self.new_folder_entry.delete(0, "end")
        self.subfolder_combo.set(folder_name)
        self.hide_new_folder_fields()

        messagebox.showinfo("Test Created", f"Created test file:\n{relative_path}")

    def on_test_selected(self, event: tk.Event) -> None:
        selected = self.test_list_combo.get()
        if not selected:
            return
        test_path = TESTS_DIR / selected
        if not test_path.exists():
            messagebox.showerror("File Missing", f"Test file not found:\n{selected}")
            return

        metadata = self.metadata.get(selected, {})
        if not metadata:
            content = read_test_file(test_path)
            metadata = parse_metadata_from_content(content)

        self.result_entry.delete(0, "end")
        self.result_entry.insert(0, metadata.get("Result", ""))
        self.notes_text.delete("1.0", "end")
        self.notes_text.insert("1.0", metadata.get("Notes", ""))

    def on_update_result(self) -> None:
        selected = self.test_list_combo.get()
        result_text = self.result_entry.get().strip()
        notes = self.notes_text.get("1.0", "end").strip()

        if not selected:
            messagebox.showwarning("No Test Selected", "Please choose a test file to update.")
            return
        if not result_text:
            messagebox.showwarning("Missing Result", "Please enter a test result.")
            return

        test_path = TESTS_DIR / selected
        if not test_path.exists():
            messagebox.showerror("File Missing", f"Test file not found:\n{selected}")
            return

        content = read_test_file(test_path)
        metadata = self.metadata.get(selected, {})
        if not metadata:
            metadata = parse_metadata_from_content(content)

        metadata["Result"] = result_text
        metadata["Notes"] = notes
        metadata["Last Updated"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        metadata["Path"] = selected.replace('\\', '/')

        updated = append_or_replace_metadata_block(content, metadata)
        write_test_file(test_path, updated)

        self.metadata[selected] = metadata
        save_metadata(self.metadata)
        messagebox.showinfo("Result Saved", f"Updated result for: {selected}")

    def refresh_test_list(self) -> None:
        self.test_list_combo.config(values=self.get_test_list())


def parse_metadata_from_content(content: str) -> dict:
    metadata = {}
    start = content.find(METADATA_START)
    end = content.find(METADATA_END, start + len(METADATA_START))
    if start == -1 or end == -1:
        return metadata
    block = content[start:end + len(METADATA_END)].splitlines()
    for line in block:
        cleaned = line.strip().lstrip("/").strip()
        if ":" in cleaned:
            key, value = cleaned.split(":", 1)
            metadata[key.strip()] = value.strip()
    return metadata


def main() -> None:
    ensure_tests_dir()
    app = TestCreatorApp()
    app.mainloop()


if __name__ == "__main__":
    main()
