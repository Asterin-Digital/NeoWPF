import ctypes
import os
import sys
import winreg
from pathlib import Path


def is_admin() -> bool:
    try:
        return bool(ctypes.windll.shell32.IsUserAnAdmin())
    except AttributeError:
        return False


def relaunch_as_admin() -> int:
    class ShellExecuteInfo(ctypes.Structure):
        _fields_ = [
            ("cbSize", ctypes.c_ulong),
            ("fMask", ctypes.c_ulong),
            ("hwnd", ctypes.c_void_p),
            ("lpVerb", ctypes.c_wchar_p),
            ("lpFile", ctypes.c_wchar_p),
            ("lpParameters", ctypes.c_wchar_p),
            ("lpDirectory", ctypes.c_wchar_p),
            ("nShow", ctypes.c_int),
            ("hInstApp", ctypes.c_void_p),
            ("lpIDList", ctypes.c_void_p),
            ("lpClass", ctypes.c_wchar_p),
            ("hkeyClass", ctypes.c_void_p),
            ("dwHotKey", ctypes.c_ulong),
            ("hIcon", ctypes.c_void_p),
            ("hProcess", ctypes.c_void_p),
        ]

    script = Path(__file__).resolve()
    parameters = " ".join(f'"{argument}"' for argument in [script, *sys.argv[1:]])
    execute_info = ShellExecuteInfo(
        ctypes.sizeof(ShellExecuteInfo),
        0x00000040,
        None,
        "runas",
        sys.executable,
        parameters,
        str(script.parent),
        1,
    )
    if not ctypes.windll.shell32.ShellExecuteExW(ctypes.byref(execute_info)):
        raise OSError("Unable to request administrator privileges")

    ctypes.windll.kernel32.WaitForSingleObject(execute_info.hProcess, 0xFFFFFFFF)
    exit_code = ctypes.c_ulong()
    ctypes.windll.kernel32.GetExitCodeProcess(
        execute_info.hProcess,
        ctypes.byref(exit_code),
    )
    ctypes.windll.kernel32.CloseHandle(execute_info.hProcess)
    return exit_code.value


def find_x64_compiler_directory() -> Path:
    roots = [
        Path(value)
        for value in (os.environ.get("ProgramFiles"), os.environ.get("ProgramFiles(x86)"))
        if value
    ]
    compiler = next(
        (
            path
            for root in roots
            for path in root.glob(
                "Microsoft Visual Studio/**/VC/Tools/MSVC/**/bin/HostX64/x64/cl.exe"
            )
            if path.is_file()
        ),
        None,
    )
    if compiler is None:
        raise FileNotFoundError("An x64 MSVC compiler was not found")
    return compiler.parent


def add_to_system_path(compiler_directory: Path) -> None:
    key_path = r"SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
    with winreg.OpenKey(
        winreg.HKEY_LOCAL_MACHINE,
        key_path,
        0,
        winreg.KEY_READ | winreg.KEY_WRITE,
    ) as key:
        system_path, value_type = winreg.QueryValueEx(key, "Path")
        entries = [entry for entry in system_path.split(os.pathsep) if entry]
        directory_text = str(compiler_directory)
        if directory_text.casefold() not in {entry.casefold() for entry in entries}:
            entries.append(directory_text)
            winreg.SetValueEx(key, "Path", 0, value_type, os.pathsep.join(entries))

    ctypes.windll.user32.SendMessageTimeoutW(
        0xFFFF,
        0x001A,
        0,
        "Environment",
        0x0002,
        5000,
        None,
    )
    print("Updated System Path")


def verify_system_path(compiler_directory: Path) -> None:
    key_path = r"SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key_path, 0, winreg.KEY_READ) as key:
        system_path, _ = winreg.QueryValueEx(key, "Path")
    if str(compiler_directory).casefold() not in {
        entry.casefold() for entry in system_path.split(os.pathsep)
    }:
        raise RuntimeError("System Path verification failed: compiler directory is missing")

    print("SUCCESS: System Path verified")


def main() -> None:
    if not is_admin():
        sys.exit(relaunch_as_admin())

    try:
        compiler_directory = find_x64_compiler_directory()
        add_to_system_path(compiler_directory)
        verify_system_path(compiler_directory)
        ctypes.windll.user32.MessageBoxW(
            None,
            "System Path and MSVC environment verified successfully.",
            "NeoWPF System Path",
            0x40,
        )
    except Exception as error:
        ctypes.windll.user32.MessageBoxW(
            None,
            str(error),
            "NeoWPF System Path Error",
            0x10,
        )
        raise


if __name__ == "__main__":
    main()
