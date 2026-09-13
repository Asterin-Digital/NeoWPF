import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent  


def collect(root: Path):
    src = root / "src"
    inc = root / "include"

    cpp    = sorted(
        p for p in src.rglob("*.cpp")
        if p.is_file() and "Rendering/D3D11" not in p.relative_to(root).as_posix()
        and "Rendering/Vulkan" not in p.relative_to(root).as_posix()
    )
    h      = sorted(p for p in inc.rglob("*.h")   if p.is_file())
    d3d11  = sorted(p for p in (src / "Rendering" / "D3D11").rglob("*.cpp") if p.is_file())
    vulkan = sorted(p for p in (src / "Rendering" / "Vulkan").rglob("*.cpp") if p.is_file())

    return cpp, h, d3d11, vulkan


def format_list(name: str, files, root: Path) -> str:
    lines = [f"set({name}"]
    for f in files:
        lines.append(f'    "${{CMAKE_CURRENT_SOURCE_DIR}}/{f.relative_to(root).as_posix()}"')
    lines.append(")")
    return "\n".join(lines)


def main():
    if not ROOT.is_dir():
        sys.exit(f"Root not found: {ROOT}")

    cpp, headers, d3d11, vulkan = collect(ROOT)
    out = ROOT / "cmake" / "NeoWPFSources.cmake"

    parts = [
        format_list("NEOWPF_SOURCES", cpp, ROOT),
        "",
        format_list("NEOWPF_BACKEND_SOURCES_D3D11", d3d11, ROOT),
        "",
        format_list("NEOWPF_BACKEND_SOURCES_VULKAN", vulkan, ROOT),
        "",
        format_list("NEOWPF_HEADERS", headers, ROOT),
        "",
        'list(FILTER NEOWPF_SOURCES EXCLUDE REGEX "/src/NeoNaml/")',
        'list(FILTER NEOWPF_SOURCES EXCLUDE REGEX "/src/Media/")',
        "",
        "if(NOT WIN32)",
        '    list(FILTER NEOWPF_SOURCES EXCLUDE REGEX "/src/Platform/Windows/")',
        "else()",
        '    list(FILTER NEOWPF_SOURCES EXCLUDE REGEX "/src/Platform/Linux/")',
        "endif()",
        "",
        'if(NEOWPF_RENDERER_BACKEND STREQUAL "D3D11")',
        "    set(NEOWPF_BACKEND_SOURCES ${NEOWPF_BACKEND_SOURCES_D3D11})",
        'elseif(NEOWPF_RENDERER_BACKEND STREQUAL "Vulkan")',
        "    set(NEOWPF_BACKEND_SOURCES ${NEOWPF_BACKEND_SOURCES_VULKAN})",
        "else()",
        "    message(FATAL_ERROR",
        '        "Unknown NEOWPF_RENDERER_BACKEND=\'${NEOWPF_RENDERER_BACKEND}\'. Expected \'D3D11\' or \'Vulkan\'.")',
        "endif()",
        "",
        "if(NOT NEOWPF_BACKEND_SOURCES)",
        "    message(FATAL_ERROR",
        '        "No sources found for NEOWPF_RENDERER_BACKEND=\'${NEOWPF_RENDERER_BACKEND}\'.")',
        "endif()",
        "",
        "list(APPEND NEOWPF_SOURCES ${NEOWPF_BACKEND_SOURCES})",
    ]

    out.write_text("\n".join(parts) + "\n", encoding="utf-8")

    print(f"Wrote {out}")
    print(f"  NEOWPF_SOURCES               : {len(cpp)}")
    print(f"  NEOWPF_BACKEND_SOURCES_D3D11 : {len(d3d11)}")
    print(f"  NEOWPF_BACKEND_SOURCES_VULKAN: {len(vulkan)}")
    print(f"  NEOWPF_HEADERS               : {len(headers)}")


if __name__ == "__main__":
    main()