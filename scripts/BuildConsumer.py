import subprocess
import sys
from pathlib import Path

base = Path(r"C:\Users\Katsu\source\repos\Pythonpython")
project = base / "Pythonpython.vcxproj"
exe = base / "x64" / "Debug" / "Pythonpython.exe"

print("building...")

result = subprocess.run([
    "msbuild",
    str(project),
    "/p:Configuration=Debug",
    "/p:Platform=x64",
    "/m"
])

if result.returncode != 0:
    print("\nbuild failed.")
    sys.exit(result.returncode)

print("\nbuild succeeded.")

if not exe.exists():
    print(f"\nERROR: executable not found:")
    print(exe)
    sys.exit(1)

print(f"running: {exe}\n")

subprocess.Popen(
    [str(exe)],
    cwd=str(base)
)