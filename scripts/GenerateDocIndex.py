import argparse
import json
import re
from pathlib import Path
from typing import List, Dict, Optional


COMMENT_START = "/*"
COMMENT_END = "*/"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a JSON index from documented NeoWPF headers")
    parser.add_argument("--repo-root", default=str(Path(__file__).resolve().parent.parent), help="Path to the NeoWPF repository root")
    parser.add_argument("--include-root", default=None, help="Optional include root to scan")
    parser.add_argument("--output", default=None, help="Path to write the JSON index")
    return parser.parse_args()


def strip_comment_markers(comment: str) -> str:
    lines = []
    for raw_line in comment.splitlines():
        line = raw_line.strip()
        if not line:
            lines.append("")
            continue
        if line.startswith("/*") or line.startswith("*/"):
            continue
        if line.startswith("*"):
            line = line[1:].lstrip()
        lines.append(line)
    return "\n".join(lines)


def parse_doc_comment(comment: str) -> Dict[str, object]:
    cleaned = strip_comment_markers(comment)
    sections: Dict[str, List[object]] = {
        "summary": [],
        "details": [],
        "example": [],
        "params": [],
        "devwarn": [],
    }
    is_internal = False

    current_tag: Optional[str] = None
    current_param: Optional[Dict[str, object]] = None
    current_devwarn: Optional[Dict[str, object]] = None

    for raw_line in cleaned.splitlines():
        line = raw_line.strip()
        if not line:
            if current_tag == "example":
                sections["example"].append("")
            continue

        if line.startswith("@"):
            tag = line[1:].split()[0]
            current_tag = tag
            current_param = None
            current_devwarn = None

            if tag == "internal":
                is_internal = True
                continue

            remainder = line[len(tag) + 1:].strip()
            if tag == "param":
                param_name = remainder.split()[0] if remainder else ""
                current_param = {"name": param_name, "text": []}
                sections["params"].append(current_param)
            elif tag == "devwarn":
                code = ""
                summary = ""
                if remainder:
                    parts = remainder.split(maxsplit=1)
                    code = parts[0]
                    summary = parts[1] if len(parts) > 1 else ""
                current_devwarn = {"code": code, "summary": summary, "details": []}
                sections["devwarn"].append(current_devwarn)
            elif remainder:
                sections.setdefault(tag, []).append(remainder)
            continue

        if current_tag == "param" and current_param is not None:
            current_param["text"].append(line)
        elif current_tag == "devwarn" and current_devwarn is not None:
            current_devwarn["details"].append(line)
        elif current_tag:
            sections.setdefault(current_tag, []).append(line)

    def join_block(block: List[str]) -> str:
        return "\n".join(block).strip()

    params = []
    for entry in sections["params"]:
        text = " ".join(entry["text"]).strip()
        if entry["name"] or text:
            params.append({"name": entry["name"], "text": text})

    dev_warnings = []
    for entry in sections["devwarn"]:
        details = " ".join(entry["details"]).strip()
        dev_warnings.append({"code": entry["code"], "summary": entry["summary"], "details": details})

    return {
        "summary": join_block(sections["summary"]),
        "details": join_block(sections["details"]),
        "example": join_block(sections["example"]),
        "params": params,
        "devWarnings": dev_warnings,
        "isInternal": is_internal,
    }


def extract_decl_name(line: str) -> Optional[str]:
    stripped = line.strip()
    if not stripped or stripped.startswith("using ") or stripped.startswith("public:") or stripped.startswith("private:") or stripped.startswith("protected:"):
        return None

    if stripped.startswith("class "):
        match = re.match(r"class\s+([A-Za-z_][A-Za-z0-9_]*)", stripped)
        return match.group(1) if match else None

    if stripped.startswith("struct "):
        match = re.match(r"struct\s+([A-Za-z_][A-Za-z0-9_]*)", stripped)
        return match.group(1) if match else None

    if stripped.startswith("enum "):
        match = re.match(r"enum(?:\s+(?:class|struct))?\s+([A-Za-z_][A-Za-z0-9_]*)", stripped)
        return match.group(1) if match else None

    if "(" in stripped:
        match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(", stripped)
        if match:
            return match.group(1)

    if "=" in stripped and stripped.endswith(";"):
        match = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*=", stripped)
        if match:
            return match.group(1)

    return None


def get_declaration_kind(line: str) -> str:
    stripped = line.strip()
    if stripped.startswith("class "):
        return "class"
    if stripped.startswith("struct "):
        return "struct"
    if stripped.startswith("enum "):
        return "enum"
    if stripped.startswith("using "):
        return "using"
    if "(" in stripped and not stripped.startswith("if") and not stripped.startswith("for") and not stripped.startswith("while") and not stripped.startswith("switch"):
        if stripped.startswith("operator"):
            return "operator"
        if stripped.endswith("= default;") or stripped.endswith("= delete;"):
            return "constructor"
        return "method"
    if "=" in stripped and stripped.endswith(";"):
        return "field"
    return "declaration"


def find_namespace(content: str) -> str:
    match = re.search(r"namespace\s+([A-Za-z0-9_:]+)", content)
    return match.group(1) if match else ""


def parse_header(path: Path, repo_root: Path) -> List[Dict[str, object]]:
    content = path.read_text(encoding="utf-8")
    namespace = find_namespace(content)
    entries: List[Dict[str, object]] = []
    lines = content.splitlines()

    for index, line in enumerate(lines):
        stripped = line.strip()
        if not stripped.startswith("/*"):
            continue

        comment_start = index
        comment_end = index
        while comment_end + 1 < len(lines) and "*/" not in lines[comment_end]:
            comment_end += 1

        if comment_end >= len(lines):
            break

        comment_lines = lines[comment_start:comment_end + 1]
        comment_text = "\n".join(comment_lines)
        doc = parse_doc_comment(comment_text)
        if not doc["summary"] and not doc["details"] and not doc["example"]:
            continue

        next_index = comment_end + 1
        while next_index < len(lines) and not lines[next_index].strip():
            next_index += 1

        if next_index >= len(lines):
            continue

        decl_line = lines[next_index].strip()
        decl_name = extract_decl_name(decl_line)
        kind = get_declaration_kind(decl_line)
        if not decl_name or kind == "using":
            continue

        source_name = path.stem
        entries.append({
            "kind": kind,
            "name": decl_name,
            "source": source_name,
            "summary": doc["summary"],
            "details": doc["details"],
            "example": doc["example"],
            "params": doc["params"],
            "devWarnings": doc["devWarnings"],
            "isInternal": doc["isInternal"],
        })

    return entries


def collect_headers(include_root: Path) -> List[Path]:
    headers = []
    for path in include_root.rglob("*.h"):
        headers.append(path)
    for path in include_root.rglob("*.hpp"):
        headers.append(path)
    return sorted(headers)


def build_index(repo_root: Path, include_root: Path) -> Dict[str, object]:
    entries: List[Dict[str, object]] = []
    for header_path in collect_headers(include_root):
        entries.extend(parse_header(header_path, repo_root))

    entries.sort(key=lambda item: (item["source"], item["name"]))
    return {
        "entries": entries,
    }


def main() -> None:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    include_root = Path(args.include_root).resolve() if args.include_root else (repo_root / "include" / "NeoWPF")
    output_path = Path(r"E:\NeoWPF.doc\public\content\api\api-index.json")

    index = build_index(repo_root, include_root)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(index, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Wrote {len(index['entries'])} entries to {output_path}")


if __name__ == "__main__":
    main()
