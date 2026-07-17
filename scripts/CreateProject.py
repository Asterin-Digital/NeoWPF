#!/usr/bin/env python3
import os
import re
import sys

def ask_yes_no(prompt):
    while True:
        ans = input(prompt + " [Y/N]: ").strip().lower()
        if ans in ('y', 'yes'):
            return True
        if ans in ('n', 'no'):
            return False
        print("Please answer Y or N.")

def camel_to_title(name):
    if not name:
        return "MyApp"
    return re.sub(r'(?<=[a-z])(?=[A-Z])', ' ', name)

def write_file(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)

def main():
    print("NeoWPF Project Generator")
    if not ask_yes_no("Are you sure you're running this script at the root of your workspace?"):
        print("Aborted.")
        sys.exit(0)

    project_name = input("What's the name of your project? (You can skip this by not typing anything): ").strip()
    if project_name:
        include_root = os.path.join("include", project_name)
        include_header = f"<{project_name}/MainWindow.h>"
        window_title = camel_to_title(project_name)
    else:
        include_root = "include"
        include_header = "<MainWindow.h>"
        window_title = "MyApp"

    os.makedirs("src", exist_ok=True)
    os.makedirs(include_root, exist_ok=True)

    main_window_h = """#pragma once

#include <NeoWPF/NeoWPF.h>
#include <memory>

std::shared_ptr<NeoWPF::UI::Grid> CreateMainContent();
"""
    write_file(os.path.join(include_root, "MainWindow.h"), main_window_h)

    main_window_cpp = f"""#include {include_header}
#include <NeoWPF/NeoWPF.h>

std::shared_ptr<NeoWPF::UI::Grid> CreateMainContent() {{
    using namespace NeoWPF;
    using namespace NeoWPF::UI;

    auto root = std::make_shared<Grid>();
    root->SetBackgroundColor(0xFF0B1220u);
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);

    auto card = std::make_shared<Border>();
    card->SetWidth(520);
    card->SetHeight(160);
    card->SetPadding(16);
    card->SetCornerRadius(CornerRadius(14.0f));
    card->SetBorderThickness(1);
    card->SetBackgroundColor(0xFF0F172Au);
    card->SetBorderColor(0xFF334155u);
    card->SetHorizontalAlignment(HorizontalAlignment::Center);
    card->SetVerticalAlignment(VerticalAlignment::Center);

    auto text = std::make_shared<TextBlock>();
    text->SetText(L"Welcome to {window_title}");
    text->SetTextColor(0xFFEDF2F7u);
    text->SetFontSize(24);

    card->SetChild(text);
    root->AddChild(card, 0, 0);

    return root;
}}
"""
    write_file(os.path.join("src", "MainWindow.cpp"), main_window_cpp)

    app_cpp = f"""#include {include_header}
#include <NeoWPF/NeoWPF.h>

int main() {{
    NeoWPF::Core::Application::Initialize();

    auto content = CreateMainContent();

    NeoWPF::Core::Application::Current()->CreateWindow(L"{window_title}", 900, 680);

    return NeoWPF::Core::Application::Run();
}}
"""
    write_file(os.path.join("src", "App.cpp"), app_cpp)

    print("\nProject generated.")
    print(f"  src/App.cpp")
    print(f"  src/MainWindow.cpp")
    print(f"  {include_root}/MainWindow.h")

if __name__ == "__main__":
    main()