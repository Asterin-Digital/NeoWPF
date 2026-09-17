#include <NeoCM/Core/Compiler/CppEmitter.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace NeoCM::Core::Compiler {
namespace {

struct ElementRecord {
    const ResolvedXamlElement* element = nullptr;
    std::string variable;
    std::string builder;
    std::size_t parent = static_cast<std::size_t>(-1);
    std::vector<std::size_t> childIndices;
    std::string resourceKey;
    std::size_t resourceOwner = 0;
    bool isResource = false;
    bool isImplicitStyle = false;
};

class CppWriter {
public:
    void Line(const std::string& value = {})
    {
        m_output << value << '\n';
    }

    void BlankLine()
    {
        Line();
    }

    std::string Str() const
    {
        return m_output.str();
    }

private:
    std::ostringstream m_output;
};

std::string Identifier(std::string value)
{
    for (char& character : value) {
        if (!std::isalnum(static_cast<unsigned char>(character)) && character != '_') {
            character = '_';
        }
    }
    if (value.empty() || std::isdigit(static_cast<unsigned char>(value.front()))) {
        value.insert(value.begin(), '_');
    }
    return value;
}

std::string StringLiteral(const std::string& value)
{
    std::ostringstream result;
    result << '"';
    for (const char character : value) {
        switch (character) {
        case '\\': result << "\\\\"; break;
        case '"': result << "\\\""; break;
        case '\n': result << "\\n"; break;
        case '\r': result << "\\r"; break;
        case '\t': result << "\\t"; break;
        default: result << character; break;
        }
    }
    result << '"';
    return result.str();
}

bool IsRootSubclass(const ResolvedXamlDocument& document)
{
    return document.rootKind != NeoWPF::UI::Markup::Xaml::XamlTypeRole::Element;
}

void CollectElements(const ResolvedXamlElement& element,
                     std::vector<ElementRecord>& records,
                     std::string& error,
                     std::size_t parent = static_cast<std::size_t>(-1))
{
    const auto index = records.size();
    records.push_back({&element, "element" + std::to_string(index), "builder" + std::to_string(index), parent, {}, {}, 0, false, false});
    for (const auto& child : element.children) {
        const auto childIndex = records.size();
        CollectElements(child.element, records, error, index);
        if (!error.empty()) {
            return;
        }
        records[index].childIndices.push_back(childIndex);
    }
    for (const auto& resource : element.resources) {
        const auto resourceIndex = records.size();
        CollectElements(resource.element, records, error, index);
        if (!error.empty()) return;
        records[resourceIndex].resourceKey = resource.key;
        records[resourceIndex].resourceOwner = index;
        records[resourceIndex].isResource = true;
        records[resourceIndex].isImplicitStyle = resource.isImplicitStyle;
    }
}

std::vector<std::string> CollectHeaders(const ResolvedXamlDocument& document,
                                        const std::vector<ElementRecord>& records)
{
    std::unordered_set<std::string> headers;
    headers.insert(document.root.headerPath);
    for (const auto& record : records) {
        headers.insert(record.element->headerPath);
    }

    std::vector<std::string> sortedHeaders(headers.begin(), headers.end());
    std::sort(sortedHeaders.begin(), sortedHeaders.end());
    return sortedHeaders;
}

void EmitHeader(const ResolvedXamlDocument& document,
                const CppEmitterOptions& options,
                const std::vector<ElementRecord>& records,
                CppWriter& output)
{
    output.Line("#pragma once");
    output.BlankLine();
    output.Line("#include <NeoWPF/NeoWPF.h>");
    for (const auto& header : CollectHeaders(document, records)) {
        output.Line("#include <" + header + ">");
    }
    output.BlankLine();
    output.Line("namespace " + options.namespaceName + " {");
    output.BlankLine();
    const auto className = document.xamlClass.empty()
        ? options.componentName
        : document.xamlClass.substr(document.xamlClass.find_last_of('.') + 1);
    if (IsRootSubclass(document)) {
        output.Line("class " + className + " : public " + document.root.cppTypeName + " {");
    } else {
        output.Line("class " + className + " {");
    }
    output.Line("public:");
    if (IsRootSubclass(document)) {
        output.Line("    " + className + "();");
    }
    if (!IsRootSubclass(document)) {
        output.Line("    std::shared_ptr<" + records.front().element->cppTypeName + "> Root;");
    }
    for (std::size_t index = 1; index < records.size(); ++index) {
        const auto& element = *records[index].element;
        if (!element.xamlName.empty()) {
            output.Line("    std::shared_ptr<" + element.cppTypeName + "> " + Identifier(element.xamlName) + ";");
        }
    }
    output.BlankLine();
    output.Line("    void InitializeComponent();");
    output.Line("};");
    output.BlankLine();
    output.Line("}");
}

void EmitElementConstruction(const ElementRecord& record,
                             bool isRootSubclass,
                             CppWriter& output)
{
    if (isRootSubclass && record.parent == static_cast<std::size_t>(-1)) {
        output.Line("    auto builder0 = NeoWPF::UI::Builder::ElementBuilder<" +
                    record.element->cppTypeName + ">(std::shared_ptr<" +
                    record.element->cppTypeName + ">(this, [](" +
                    record.element->cppTypeName + "*) {}));");
        output.Line("    auto element0 = builder0.Build();");
        return;
    }
    output.Line("    auto " + record.builder + " = NeoWPF::UI::Builder::Build<" +
                record.element->cppTypeName + ">();");
    if (!record.element->xamlName.empty()) {
        output.Line("    " + record.builder + ".Name(\"" + record.element->xamlName + "\");");
    }
    output.Line("    auto " + record.variable + " = " + record.builder + ".Build();");
}

void EmitProperties(const ElementRecord& record, CppWriter& output)
{
    for (const auto& property : record.element->properties) {
        if (property.isMarkupExtension) {
            continue;
        }
        output.Line("    " + record.builder + ".Set(" + property.propertyExpression + ", " +
                    property.valueExpression + ");");
    }
}

const ElementRecord* FindResource(const std::vector<ElementRecord>& records,
                                  std::size_t elementIndex,
                                  const std::string& key)
{
    for (auto scope = elementIndex;
         scope != static_cast<std::size_t>(-1);
         scope = records[scope].parent) {
        const auto resource = std::find_if(records.begin(), records.end(), [scope, &key](const auto& candidate) {
            return candidate.isResource && candidate.resourceOwner == scope && candidate.resourceKey == key;
        });
        if (resource != records.end()) {
            return &*resource;
        }
    }
    return nullptr;
}

void EmitBindings(const ElementRecord& record,
                  std::size_t recordIndex,
                  const std::vector<ElementRecord>& records,
                  CppWriter& output)
{
    for (const auto& property : record.element->properties) {
        if (!property.isMarkupExtension) {
            continue;
        }
        if (!property.staticResourceKey.empty()) {
            const auto* resource = FindResource(records, recordIndex, property.staticResourceKey);
            if (!resource) {
                continue;
            }
            output.Line("    " + record.builder + ".Assign(" + property.propertyExpression + ", " +
                        resource->variable + ");");
            continue;
        }
        if (!property.localeKey.empty()) {
            output.Line("    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*" + record.variable + ", " +
                        property.propertyExpression + ", NeoWPF::Platform::Utf8ToWide(" +
                        StringLiteral(property.localeKey) + "));" );
            continue;
        }
        const auto mode = property.bindingMode.empty() ? "OneWay" : property.bindingMode;
        output.Line("    NeoWPF::UI::Data::BindingOperations::SetBinding(*" + record.variable + ", " +
                    property.propertyExpression + ", NeoWPF::UI::Data::Binding(" +
                    StringLiteral(property.bindingPath) + ", NeoWPF::UI::Data::BindingMode::" + mode + "));" );
    }
}

void EmitResources(const std::vector<ElementRecord>& records, CppWriter& output)
{
    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        if (record.isResource) {
            for (std::size_t childPosition = 0;
                 childPosition < record.childIndices.size();
                 ++childPosition) {
                const auto& child = record.element->children[childPosition];
                if (!child.collectionAdder.empty()) {
                    output.Line("    element" + std::to_string(index) +
                                "->" + child.collectionAdder + "(element" +
                                std::to_string(record.childIndices[childPosition]) + ");");
                }
            }
            if (record.isImplicitStyle) {
                output.Line("    element" + std::to_string(record.resourceOwner) +
                            "->SetImplicitXamlStyle(element" + std::to_string(index) +
                            "->GetTargetType(), element" + std::to_string(index) + ");");
            } else {
                output.Line("    element" + std::to_string(record.resourceOwner) +
                            "->SetResource(" + StringLiteral(record.resourceKey) + ", element" +
                            std::to_string(index) + ");");
            }
        }
    }
}

void EmitChildAttachments(const std::vector<ElementRecord>& records, CppWriter& output)
{
    for (std::size_t recordIndex = 0; recordIndex < records.size(); ++recordIndex) {
        const auto& record = records[recordIndex];
        for (std::size_t childPosition = 0; childPosition < record.childIndices.size(); ++childPosition) {
            const auto childIndex = record.childIndices[childPosition];
            const auto& child = record.element->children[childPosition];
            if (child.propertyName.empty()) {
                output.Line("    " + record.builder + ".AddChild(element" + std::to_string(childIndex) + ");");
            } else if (!child.collectionAdder.empty()) {
                if (record.isResource) {
                    continue;
                }
                output.Line("    element" + std::to_string(recordIndex) + "->" + child.collectionAdder +
                            "(element" + std::to_string(childIndex) + ");");
            } else {
                output.Line("    " + record.builder + ".Assign(" + child.propertyExpression +
                            ", element" + std::to_string(childIndex) + ");");
            }
        }
    }
}

void EmitNamedElementAssignments(const std::vector<ElementRecord>& records,
                                 bool isRootSubclass,
                                 CppWriter& output)
{
    if (!isRootSubclass) {
        output.Line("    Root = element0;");
    }
    for (std::size_t index = 1; index < records.size(); ++index) {
        const auto& element = *records[index].element;
        if (!element.xamlName.empty()) {
            output.Line("    " + Identifier(element.xamlName) + " = element" + std::to_string(index) + ";");
        }
    }
}

void EmitSource(const CppEmitterOptions& options,
                const ResolvedXamlDocument& document,
                const std::vector<ElementRecord>& records,
                CppWriter& output)
{
    const auto className = document.xamlClass.empty()
        ? options.componentName
        : document.xamlClass.substr(document.xamlClass.find_last_of('.') + 1);
    const auto isRootSubclass = IsRootSubclass(document);
    output.Line("#include <" + options.headerInclude + ">");
    output.Line("#include <NeoWPF/Platform/PlatformUtils.h>");
    output.BlankLine();
    output.Line("namespace " + options.namespaceName + " {");
    output.BlankLine();
    output.Line("void " + className + "::InitializeComponent()");
    output.Line("{");
    const std::string initializationGuard = isRootSubclass
        ? "IsXamlComponentInitialized()"
        : "Root";
    output.Line("    if (" + initializationGuard + ") {");
    output.Line("        return;");
    output.Line("    }");
    output.BlankLine();
    for (const auto& record : records) {
        EmitElementConstruction(record, isRootSubclass, output);
    }
    output.BlankLine();
    for (const auto& record : records) {
        EmitProperties(record, output);
    }
    output.BlankLine();
    EmitResources(records, output);
    for (std::size_t index = 0; index < records.size(); ++index) {
        const auto& record = records[index];
        if (record.isResource) {
            continue;
        }
        EmitBindings(record, index, records, output);
    }
    output.BlankLine();
    EmitChildAttachments(records, output);
    output.BlankLine();
    EmitNamedElementAssignments(records, isRootSubclass, output);
    if (isRootSubclass) {
        output.Line("    MarkXamlComponentInitialized();");
    }
    output.Line("}");
    output.BlankLine();
    output.Line("}");
}

}

CppGenerationResult EmitCppComponent(const ResolvedXamlDocument& document,
                                     const CppEmitterOptions& options)
{
    CppGenerationResult result;
    if (options.namespaceName.empty() || options.componentName.empty() || options.headerInclude.empty()) {
        result.error = "C++ emitter options require namespaceName, componentName, and headerInclude";
        return result;
    }

    std::vector<ElementRecord> records;
    CollectElements(document.root, records, result.error);
    if (!result.error.empty()) {
        return result;
    }

    std::unordered_set<std::string> fieldNames;
    fieldNames.insert("Root");
    for (std::size_t index = 1; index < records.size(); ++index) {
        const auto& name = records[index].element->xamlName;
        if (name.empty()) {
            continue;
        }
        const auto fieldName = Identifier(name);
        if (!fieldNames.insert(fieldName).second) {
            result.error = "XAML names produce duplicate C++ field name: " + fieldName;
            return result;
        }
    }

    CppWriter header;
    EmitHeader(document, options, records, header);

    CppWriter source;
    EmitSource(options, document, records, source);

    result.success = true;
    result.header = header.Str();
    result.source = source.Str();
    return result;
}

}