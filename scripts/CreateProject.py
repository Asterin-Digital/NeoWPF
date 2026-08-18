import shutil
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

MSBUILD_NS = "http://schemas.microsoft.com/developer/msbuild/2003"
ET.register_namespace("", MSBUILD_NS)

project = Path(r"C:\Users\Katsu\source\repos\Pythonpython")
neopm = Path(r"C:\Users\Katsu\source\repos\NeoWPF\x64\Debug\NeoPM.exe")
neo_wpf_include = r"C:\Users\Katsu\source\repos\NeoWPF\include"
neo_wpf_lib = r"C:\Users\Katsu\source\repos\NeoWPF\x64\Debug\NeoWPF.lib"
vcxproj_path = project / "Pythonpython.vcxproj"

if project.exists():
    print(f"Deleting {project}...")
    shutil.rmtree(project)

print("Creating project...")
subprocess.run(
    [str(neopm), "new", "Pythonpython"],
    check=True,
)

print("Patching generated project...")

def ensure_namespaced_child(parent, tag_name):
    tag = f"{{{MSBUILD_NS}}}{tag_name}"
    child = parent.find(tag)
    if child is None:
        child = ET.SubElement(parent, tag)
    return child


def normalize_list_text(value):
    if not value:
        return []
    return [part.strip() for part in value.split(";") if part.strip()]


def append_path_value(parent, item_name, value, macro_name):
    child = ensure_namespaced_child(parent, item_name)
    values = normalize_list_text(child.text)
    values = [part for part in values if part != macro_name]
    if value not in values:
        values.append(value)
    values.append(macro_name)
    child.text = ";".join(values)
    return child


def append_lib_value(parent, item_name, value, macro_name):
    child = ensure_namespaced_child(parent, item_name)
    values = normalize_list_text(child.text)
    values = [part for part in values if part != macro_name]
    if value not in values:
        values.append(value)
    values.append(macro_name)
    child.text = ";".join(values)
    return child


tree = ET.parse(vcxproj_path)
root = tree.getroot()
ns = {"msb": MSBUILD_NS}

for item_group in root.findall(".//msb:ItemDefinitionGroup", ns):
    cl_compile = item_group.find("msb:ClCompile", ns)
    if cl_compile is None:
        cl_compile = ET.SubElement(item_group, f"{{{MSBUILD_NS}}}ClCompile")
    append_path_value(cl_compile, "AdditionalIncludeDirectories", neo_wpf_include, "%(AdditionalIncludeDirectories)")

    link = item_group.find("msb:Link", ns)
    if link is None:
        link = ET.SubElement(item_group, f"{{{MSBUILD_NS}}}Link")
    append_lib_value(link, "AdditionalDependencies", neo_wpf_lib, "%(AdditionalDependencies)")

# Preserve the generated file as valid MSBuild XML with the default namespace.
tree.write(vcxproj_path, encoding="utf-8", xml_declaration=True)

print("Done.")