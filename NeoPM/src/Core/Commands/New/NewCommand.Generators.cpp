#include <NeoPM/Core/Commands/NewCommand.h>
#include <NeoPM/Core/Project/CMakeListsBuilder.h>
#include <NeoPM/Core/ToolchainBootstrap.h>
#include <NeoPM/Sdk/SdkManager.h>
#include <NeoPM/Utils/Logger.h>

#include <fstream>
#include <vector>

namespace NeoPM::Core::Commands {

    bool NewCommand::GenerateMainWindow(
        const std::filesystem::path& projectRoot
    ) {
        auto headerPath = projectRoot / "include" / m_projectName / "MainWindow.xaml.h";
        auto xamlPath = projectRoot / "MainWindow.xaml";
        auto sourcePath = projectRoot / "MainWindow.xaml.cpp";

        auto headerContent =
            "#pragma once\n\n"
            "#include <NeoWPF/NeoWPF.h>\n"
            "\n"
            "namespace " + m_projectName + " {\n"
            "    class MainWindow : public NeoWPF::UI::Window {\n"
            "    public:\n"
            "        void InitializeComponent();\n"
            "    };\n"
            "}\n";

        auto sourceContent =
            "#include <NeoWPF/NeoWPF.h>\n"
            "#include <" + m_projectName + "/MainWindow.xaml.h>\n\n"
            "namespace " + m_projectName + " {\n"
            "    MainWindow::MainWindow()\n"
            "    {\n"
            "        InitializeComponent();\n"
            "    }\n"
            "}\n";

        auto xamlContent =
            "<Window x:Class=\"" + m_projectName + ".MainWindow\" Title=\"" + m_projectName + "\">\n"
            "    <Window.Content>\n"
            "        <Grid />\n"
            "    </Window.Content>\n"
            "</Window>\n";

        auto headerFile = std::ofstream(headerPath);
        if (!headerFile) {
            return false;
        }
        headerFile << headerContent;
        headerFile.close();

        auto sourceFile = std::ofstream(sourcePath);
        if (!sourceFile) {
            return false;
        }
        sourceFile << sourceContent;
        sourceFile.close();

        auto xamlFile = std::ofstream(xamlPath);
        if (!xamlFile) {
            return false;
        }
        xamlFile << xamlContent;
        xamlFile.close();

        return true;
    }

    bool NewCommand::GenerateApp(
        const std::filesystem::path& projectRoot
    ) {
        auto headerPath = projectRoot / "include" / m_projectName / "App.xaml.h";
        auto xamlPath = projectRoot / "App.xaml";
        auto appPath = projectRoot / "App.xaml.cpp";

        auto headerContent =
            "#pragma once\n\n"
            "#include <NeoWPF/NeoWPF.h>\n\n"
            "namespace " + m_projectName + " {\n"
            "    class App : public NeoWPF::Core::Application {\n"
            "    public:\n"
            "        void InitializeComponent();\n"
            "    };\n"
            "}\n";

        auto appContent =
            "#include <NeoWPF/NeoWPF.h>\n"
            "#include <" + m_projectName + "/Xaml.h>\n"
            "#include <" + m_projectName + "/AssemblyInfo.h>\n"
            "#include <" + m_projectName + "/Generated/Resources.generated.h>\n\n"
            "namespace " + m_projectName + " {\n"
            "    App::App()\n"
            "    {\n"
            "        InitializeComponent();\n"
            "    }\n"
            "}\n\n"
            "int main() {\n"
            "    NeoWPF::Core::Application::Initialize();\n"
            "    auto app = std::make_shared<" + m_projectName + "::App>();\n"
            "    NeoWPF::Core::Application::SetCurrent(app);\n"
            "    auto& runtimeApp = NeoWPF::Core::Application::Current();\n\n"
            "    // Register resource providers\n"
            "    runtimeApp.Resources().RegisterProvider(\n"
            "        std::make_unique<NeoWPF::ExternalFileResourceProvider>(\n"
            "            NeoWPF::Platform::CurrentPlatform()->GetExecutableDirectory()));\n"
            "    " + m_projectName + "::Resources::RegisterAll();\n\n"
            "    auto mainWindow = std::make_shared<" + m_projectName + "::MainWindow>();\n"
            "    auto window = runtimeApp.CreateWindow(mainWindow, 900, 680);\n\n"
            "    auto result = NeoWPF::Core::Application::Run();\n"
            "    NeoWPF::Core::Application::Shutdown();\n"
            "    return result;\n"
            "}\n";

        auto xamlContent =
            "<Application x:Class=\"" + m_projectName + ".App\" />\n";

        auto appFile = std::ofstream(appPath);
        if (!appFile) {
            return false;
        }
        appFile << appContent;
        appFile.close();

        auto headerFile = std::ofstream(headerPath);
        if (!headerFile) {
            return false;
        }
        headerFile << headerContent;
        headerFile.close();

        auto xamlFile = std::ofstream(xamlPath);
        if (!xamlFile) {
            return false;
        }
        xamlFile << xamlContent;
        xamlFile.close();

        return true;
    }

    bool NewCommand::GenerateAssemblyInfo(
        const std::filesystem::path& projectRoot
    ) {
        auto headerPath = projectRoot / "include" / m_projectName / "AssemblyInfo.h";
        auto sourcePath = projectRoot / "AssemblyInfo.cpp";

        auto headerContent =
            "#pragma once\n\n"
            "namespace " + m_projectName + " {\n"
            "    namespace AssemblyInfo {\n"
            "        extern const wchar_t* Name;\n"
            "        extern const char* Version;\n"
            "        extern const char* Company;\n"
            "        extern const char* Description;\n"
            "    }\n"
            "}\n";

        auto sourceContent =
            "#include <" + m_projectName + "/AssemblyInfo.h>\n\n"
            "namespace " + m_projectName + " {\n"
            "    namespace AssemblyInfo {\n"
            "        const wchar_t* Name = L\"" + m_projectName + "\";\n"
            "        const char* Version = \"1.0.0\";\n"
            "        const char* Company = \"\";\n"
            "        const char* Description = \"\";\n"
            "    }\n"
            "}\n";

        auto headerFile = std::ofstream(headerPath);
        if (!headerFile) {
            return false;
        }
        headerFile << headerContent;
        headerFile.close();

        auto sourceFile = std::ofstream(sourcePath);
        if (!sourceFile) {
            return false;
        }
        sourceFile << sourceContent;
        sourceFile.close();

        return true;
    }

    bool NewCommand::GenerateProjectFiles(
        const std::filesystem::path& projectRoot
    ) {
        auto& logger = Utils::Logger::Instance();

        std::optional<std::filesystem::path> sdkLibPath;
        std::optional<std::filesystem::path> sdkIncludePath;

        if (m_sdkSource == "user") {
            sdkLibPath = std::filesystem::path(*m_sdkLibraryPath);
            if (std::filesystem::is_directory(sdkLibPath.value())) {
                sdkLibPath = sdkLibPath.value() / "NeoWPF.lib";
            }
            sdkIncludePath = std::filesystem::path(*m_sdkIncludePath);
            logger.Info("NewCommand", "Using user SDK (" + m_sdkConfiguration + ")");
        }
        else {
            SdkManager sdkManager;
            logger.Info("NewCommand", "Checking production NeoWPF SDK availability...");
            if (!sdkManager.RefreshSdks()) {
                logger.Error("NewCommand", "Failed to refresh SDK and no local SDKs are available");
                return false;
            }

            sdkLibPath = sdkManager.GetSdkLibraryPath(m_sdkConfiguration);
            sdkIncludePath = sdkManager.GetSdkIncludePath(m_sdkConfiguration);
        }

        if (sdkLibPath) {
            logger.Info("NewCommand", "Using SDK library: " + sdkLibPath.value().string());
        }

        if (sdkIncludePath) {
            logger.Info("NewCommand", "Using SDK include: " + sdkIncludePath.value().string());
        }

        auto cmakeModuleDir = projectRoot / "cmake";
        std::filesystem::create_directories(cmakeModuleDir);

        const auto localCMakeModulePath = cmakeModuleDir / "NeoWPFResources.cmake";
        auto localModule = std::ofstream(localCMakeModulePath, std::ios::out | std::ios::trunc);
        if (!localModule) {
            logger.Error("NewCommand", "Failed to create project-local CMake resource sync module");
            return false;
        }
        localModule << ToolchainBootstrap::BuildResourceSyncCMakeModule(ToolchainBootstrap::GetToolExecutablePath());
        localModule.close();

        Project::CMakeListsBuilder cmakeBuilder(m_projectName);
        cmakeBuilder
            .AddCompileFile("App.xaml.cpp")
            .AddCompileFile("MainWindow.xaml.cpp")
            .AddGeneratedXamlSources()
            .AddCompileFile("AssemblyInfo.cpp")
            .AddCompileFile("Generated/Resources.generated.cpp")
            .AddHeaderFile("include/" + m_projectName + "/MainWindow.xaml.h")
            .AddHeaderFile("include/" + m_projectName + "/App.xaml.h")
            .AddHeaderFile("include/" + m_projectName + "/Xaml.h")
            .AddHeaderFile("include/" + m_projectName + "/AssemblyInfo.h")
            .AddHeaderFile("include/" + m_projectName + "/Generated/Resources.generated.h")
            .AddResourceAssetGlob()
            .AddResourceSyncModule("cmake/NeoWPFResources.cmake");

        if (sdkLibPath) {
            cmakeBuilder.AddSdkLibrary(sdkLibPath.value());
        }
        if (sdkIncludePath) {
            cmakeBuilder.AddSdkIncludePath(sdkIncludePath.value());
        }

        NeoPM::Contracts::ProjectGenerationContext context{
            m_projectName,
            projectRoot,
            sdkIncludePath,
            sdkLibPath
        };

        if (!cmakeBuilder.Generate(context)) {
            logger.Error("NewCommand", "Failed to generate CMake project files");
            return false;
        }

        logger.Info("NewCommand", "Generated CMake project files");
        return true;
    }

    bool NewCommand::GenerateResourceManifest(
        const std::filesystem::path& projectRoot
    ) {
        auto manifestPath = projectRoot / "Resources" / "resources.json";

        auto content =
            "{\n"
            "  \"resources\": [],\n"
            "  \"xaml\": [\n"
            "    {\"key\": \"App\", \"path\": \"App.xaml\"},\n"
            "    {\"key\": \"MainWindow\", \"path\": \"MainWindow.xaml\"}\n"
            "  ]\n"
            "}\n";

        auto file = std::ofstream(manifestPath);
        if (!file) {
            return false;
        }

        file << content;
        file.close();

        return true;
    }

    bool NewCommand::GenerateResourceStubs(
        const std::filesystem::path& projectRoot
    ) {
        auto& logger = Utils::Logger::Instance();

        auto headerDir = projectRoot / "include" / m_projectName / "Generated";
        auto headerPath = headerDir / "Resources.generated.h";
        auto sourcePath = projectRoot / "Generated" / "Resources.generated.cpp";

        auto headerContent =
            "#pragma once\n\n"
            "#include <NeoWPF/NeoWPF.h>\n\n"
            "namespace " + m_projectName + "::Resources {\n"
            "    // Resource keys for this project\n"
            "    // Generated by NeoPM sync-resources command\n\n"
            "    // Register all resources with the ResourceManager\n"
            "    void RegisterAll();\n"
            "}\n";

        auto sourceContent =
            "#include <" + m_projectName + "/Generated/Resources.generated.h>\n\n"
            "namespace " + m_projectName + "::Resources {\n"
            "    void RegisterAll() {\n"
            "        // Resources will be registered here by sync-resources command\n"
            "    }\n"
            "}\n";

        std::filesystem::create_directories(headerDir);
        std::filesystem::create_directories(sourcePath.parent_path());

        auto headerFile = std::ofstream(headerPath, std::ios::out | std::ios::trunc);
        if (!headerFile) {
            logger.Error("NewCommand", "Failed to create Resources.generated.h");
            return false;
        }
        headerFile << headerContent;
        headerFile.close();

        auto sourceFile = std::ofstream(sourcePath, std::ios::out | std::ios::trunc);
        if (!sourceFile) {
            logger.Error("NewCommand", "Failed to create Resources.generated.cpp");
            return false;
        }
        sourceFile << sourceContent;
        sourceFile.close();

        return true;
    }

}
