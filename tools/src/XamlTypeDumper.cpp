#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::filesystem::path outputPath = std::filesystem::current_path() / "registered_xaml_types.txt";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--output" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: NeoWpfXamlTypeDumper [--output <path>]\n";
            return 0;
        }
    }

    NeoWPF::UI::Properties::RegisterCoreControlProperties();

    auto registeredTypes = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().GetRegisteredTypes();
    std::sort(registeredTypes.begin(), registeredTypes.end(),
              [](const auto& lhs, const auto& rhs) {
                  return lhs.TagName < rhs.TagName;
              });

    std::filesystem::path parent = outputPath.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream out(outputPath, std::ios::out | std::ios::trunc);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputPath << '\n';
        return 1;
    }

    out << "NeoWPF registered XAML types\n";
    out << "Output file: " << outputPath << '\n';
    out << "Total registered types: " << registeredTypes.size() << '\n';
    out << "\n";

    for (std::size_t index = 0; index < registeredTypes.size(); ++index) {
        const auto& entry = registeredTypes[index];
        const auto& info = entry.Info;

        out << "[" << index + 1 << "]\n";
        out << "  TagName: " << entry.TagName << '\n';
        out << "  CppTypeName: " << (info.CppTypeName.empty() ? "<empty>" : info.CppTypeName) << '\n';
        out << "  HeaderPath: " << (info.HeaderPath.empty() ? "<empty>" : info.HeaderPath) << '\n';
        out << "  TypeHash: " << info.Type.hash_code() << '\n';
        out << "  ContentPropertyName: " << (info.ContentPropertyName.empty() ? "<empty>" : info.ContentPropertyName) << '\n';
        out << "  DeferContentBuild: " << (info.DeferContentBuild ? "true" : "false") << '\n';
        out << "  FactoryPresent: " << (info.Factory ? "true" : "false") << '\n';
        out << "  Role: " << static_cast<int>(info.Role) << '\n';
        out << "\n";
    }

    out.flush();
    if (!out) {
        std::cerr << "Failed while writing output file: " << outputPath << '\n';
        return 1;
    }

    std::cout << "Wrote " << registeredTypes.size() << " registered XAML types to " << outputPath << '\n';
    return 0;
}
