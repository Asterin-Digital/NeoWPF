#include <NeoPM/Core/Project/VcxprojBuilder.h>
#include <NeoPM/Utils/Guid.h>

#include <fstream>
#include <sstream>

namespace NeoPM::Core::Project {

    namespace {
        std::string XmlEscape(const std::string& value) {
            std::string result;
            result.reserve(value.size());

            for (const char ch : value) {
                switch (ch) {
                case '&':
                    result += "&amp;";
                    break;

                case '<':
                    result += "&lt;";
                    break;

                case '>':
                    result += "&gt;";
                    break;

                case '"':
                    result += "&quot;";
                    break;

                case '\'':
                    result += "&apos;";
                    break;

                default:
                    result += ch;
                    break;
                }
            }

            return result;
        }
    }

    VcxprojBuilder::VcxprojBuilder(const std::string& projectName)
        : m_projectName(projectName) {
    }

    const char* VcxprojBuilder::Name() const noexcept {
      return "vcxproj";
    }

    bool VcxprojBuilder::Generate(
      const NeoPM::Contracts::ProjectGenerationContext& context
    ) {
      if (context.projectName.empty() || context.projectRoot.empty()) {
        return false;
      }

      m_projectName = context.projectName;
      m_sdkIncludePath = context.sdkIncludePath.value_or(std::filesystem::path{});
      m_sdkLibraryPath = context.sdkLibraryPath.value_or(std::filesystem::path{});

      if (!GenerateGuid()) {
        return false;
      }

      const auto projectPath = context.projectRoot / (m_projectName + ".vcxproj");
      const auto filtersPath = context.projectRoot / (m_projectName + ".vcxproj.filters");
      const auto solutionPath = context.projectRoot / (m_projectName + ".slnx");

      std::ofstream projectFile(projectPath);
      std::ofstream filtersFile(filtersPath);
      std::ofstream solutionFile(solutionPath);
      if (!projectFile || !filtersFile || !solutionFile) {
        return false;
      }

      projectFile << BuildVcxproj();
      filtersFile << BuildVcxprojFilters();
      solutionFile << BuildSlnx();
      return static_cast<bool>(projectFile) &&
           static_cast<bool>(filtersFile) &&
           static_cast<bool>(solutionFile);
    }

    VcxprojBuilder& VcxprojBuilder::AddCompileFile(const std::string& relativePath) {
        m_compileFiles.push_back(relativePath);
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddGeneratedXamlSources() {
      m_addGeneratedXamlSources = true;
      return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddHeaderFile(const std::string& relativePath) {
        m_headerFiles.push_back(relativePath);
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddTargetsImport(const std::string& targetsPath) {
        m_targetsImportPath = targetsPath;
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddResourceAssetGlob() {
        m_addResourceAssetGlob = true;
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddNeoWPFToolsDir() {
        m_addNeoWPFToolsDir = true;
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddSdkLibrary(const std::filesystem::path& libPath) {
        m_sdkLibraryPath = libPath;
        return *this;
    }

    VcxprojBuilder& VcxprojBuilder::AddSdkIncludePath(const std::filesystem::path& includePath) {
        m_sdkIncludePath = includePath;
        return *this;
    }

    bool VcxprojBuilder::GenerateGuid() {
      m_projectGuid = Utils::GenerateGuid();
        return true;
    }

    const std::string& VcxprojBuilder::GetProjectName() const noexcept {
        return m_projectName;
    }

    const std::string& VcxprojBuilder::GetProjectGuid() const noexcept {
        return m_projectGuid;
    }

    std::string VcxprojBuilder::BuildVcxproj() const {
        std::ostringstream ss;

        ss << R"xml(<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>

  <PropertyGroup Label="Globals">
    <VCProjectVersion>18.0</VCProjectVersion>
    <ProjectGuid>{)xml"
           << m_projectGuid
           << R"xml(}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
  </PropertyGroup>

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />

  <PropertyGroup Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>

  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <UseDebugLibraries>true</UseDebugLibraries>
  </PropertyGroup>

  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <UseDebugLibraries>false</UseDebugLibraries>
  </PropertyGroup>

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
)xml";

        if (!m_targetsImportPath.empty()) {
            ss << "\n  <Import Project=\"" << XmlEscape(m_targetsImportPath) << "\" />\n";
        }

        ss << R"xml(
  <ImportGroup Label="PropertySheets"
              Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props"
            Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')"
            Label="LocalAppDataPlatform" />
  </ImportGroup>

  <ImportGroup Label="PropertySheets"
              Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props"
            Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')"
            Label="LocalAppDataPlatform" />
  </ImportGroup>

  <PropertyGroup>
    <IncludePath>$(ProjectDir)\include;$(IncludePath)</IncludePath>
  </PropertyGroup>
)xml";

        if (m_addNeoWPFToolsDir) {
            ss << "\n  <PropertyGroup>\n"
               << "    <NeoWPFToolsDir>$(LocalAppData)\\NeoWPF\\</NeoWPFToolsDir>\n"
               << "  </PropertyGroup>\n";
        }

        ss << R"xml(
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <AdditionalIncludeDirectories>$(ProjectDir)\include)xml";
        
        if (!m_sdkIncludePath.empty()) {
            ss << ";" << XmlEscape(m_sdkIncludePath.string());
        }
        
        ss << R"xml(;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalDependencies>)xml";
        
        if (!m_sdkLibraryPath.empty()) {
            ss << ";" << XmlEscape(m_sdkLibraryPath.string());
        }
        
        ss << R"xml(;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>

  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <ConformanceMode>true</ConformanceMode>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <AdditionalIncludeDirectories>$(ProjectDir)\include)xml";
        
        if (!m_sdkIncludePath.empty()) {
            ss << ";" << XmlEscape(m_sdkIncludePath.string());
        }
        
        ss << R"xml(;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalDependencies>)xml";
        
        if (!m_sdkLibraryPath.empty()) {
            ss << ";" << XmlEscape(m_sdkLibraryPath.string());
        }
        
        ss << R"xml(;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>

  <ItemGroup>
)xml";

        for (const auto& file : m_compileFiles) {
            ss << "    <ClCompile Include=\""
               << XmlEscape(file)
               << "\" />\n";
        }
        if (m_addGeneratedXamlSources) {
          ss << "    <ClCompile Include=\"Generated\\**\\*.xaml.g.cpp\" />\n";
        }

        ss << R"xml(  </ItemGroup>
)xml";

        if (m_addResourceAssetGlob) {
            ss << "\n  <ItemGroup>\n"
               << "    <NeoWPFAsset Include=\"Resources\\Assets\\**\" />\n"
               << "  </ItemGroup>\n";
        }

        ss << R"xml(
  <ItemGroup>
)xml";

        for (const auto& file : m_headerFiles) {
            ss << "    <ClInclude Include=\""
               << XmlEscape(file)
               << "\" />\n";
        }

        ss << R"xml(  </ItemGroup>

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />

</Project>
)xml";

        return ss.str();
    }

    std::string VcxprojBuilder::BuildVcxprojFilters() const {
        std::ostringstream ss;

        ss << R"xml(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0"
         xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
    </Filter>

    <Filter Include="Header Files">
      <UniqueIdentifier>{93995380-89BD-4B04-88EB-625FBE52EBFB}</UniqueIdentifier>
    </Filter>
  </ItemGroup>

  <ItemGroup>
)xml";

        for (const auto& file : m_compileFiles) {
            ss << "    <ClCompile Include=\""
               << XmlEscape(file)
               << "\">\n";

            ss << "      <Filter>Source Files</Filter>\n";
            ss << "    </ClCompile>\n";
        }
          if (m_addGeneratedXamlSources) {
            ss << "    <ClCompile Include=\"Generated\\**\\*.xaml.g.cpp\">\n"
               << "      <Filter>Source Files</Filter>\n"
               << "    </ClCompile>\n";
          }

        ss << R"xml(  </ItemGroup>

  <ItemGroup>
)xml";

        for (const auto& file : m_headerFiles) {
            ss << "    <ClInclude Include=\""
               << XmlEscape(file)
               << "\">\n";

            ss << "      <Filter>Header Files</Filter>\n";
            ss << "    </ClInclude>\n";
        }

        ss << R"xml(  </ItemGroup>

</Project>
)xml";

        return ss.str();
    }

    std::string VcxprojBuilder::BuildSlnx() const {
        std::ostringstream ss;

        ss << R"xml(<Solution>
  <Configurations>
    <Platform Name="x64" />
  </Configurations>

  <Project Path=")xml"
           << XmlEscape(m_projectName)
           << R"xml(.vcxproj" Id=")xml"
           << m_projectGuid
           << R"xml(" />
</Solution>
)xml";

        return ss.str();
    }
}