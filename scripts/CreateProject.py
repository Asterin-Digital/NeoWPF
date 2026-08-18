import shutil
import subprocess
from pathlib import Path

project = Path(r"C:\Users\Katsu\source\repos\Pythonpython")
neopm = Path(r"C:\Users\Katsu\source\repos\NeoWPF\x64\Debug\NeoPM.exe")
sdk_include = Path(r"C:\Users\Katsu\source\repos\NeoWPF\include")
sdk_lib = Path(r"C:\Users\Katsu\source\repos\NeoWPF\x64\Debug")
downloads = Path(r"C:\Users\Katsu\Downloads")

if not neopm.exists():
    raise FileNotFoundError(f"NeoPM.exe not found: {neopm}")

if project.exists():
    print(f"Deleting existing project: {project}")
    shutil.rmtree(project)

print(f"Creating project at {project}...")
subprocess.run(
    [
        str(neopm),
        "new",
        "Pythonpython",
        "--sdk",
        "user",
        "--sdk-config",
        "debug",
        "--sdk-include",
        str(sdk_include),
        "--sdk-lib",
        str(sdk_lib),
    ],
    check=True,
)

main_window_cpp = project / "src" / "MainWindow.cpp"
assets_dir = project / "Resources" / "Assets"
resources_json = project / "Resources" / "resources.json"

main_window_code = r'''#include <memory>
#include <vector>
#include <NeoWPF/NeoWPF.h>
#include <Pythonpython/MainWindow.h>
#include <Pythonpython/Generated/Resources.generated.h>

namespace Pythonpython {
    class SlideshowState {
    public:
        int currentIndex = 0;
        std::vector<NeoWPF::ResourceKey> slides = {
            Resources::angel,
            Resources::dog,
            Resources::gohome,
            Resources::soft
        };
    };

    static std::shared_ptr<SlideshowState> g_slideshow;
    static std::shared_ptr<NeoWPF::UI::Image> g_currentImage;
    static std::shared_ptr<NeoWPF::UI::TextBlock> g_indexText;

    void UpdateSlide() {
        if (!g_currentImage || !g_slideshow || g_slideshow->slides.empty()) return;

        auto& resourceManager = NeoWPF::ResourceManager::Instance();
        g_currentImage->SetSource(
            resourceManager.FindImage(
                g_slideshow->slides[g_slideshow->currentIndex]
            )
        );

        if (g_indexText) {
            std::wstring index = std::to_wstring(g_slideshow->currentIndex + 1);
            std::wstring total = std::to_wstring(g_slideshow->slides.size());
            std::wstring text = index + L" / " + total;
            g_indexText->SetText(text);
        }
    }

    void OnNextClick() {
        if (!g_slideshow || g_slideshow->slides.empty()) return;

        g_slideshow->currentIndex =
            (g_slideshow->currentIndex + 1) % g_slideshow->slides.size();

        UpdateSlide();
    }

    void OnBackClick() {
        if (!g_slideshow || g_slideshow->slides.empty()) return;

        g_slideshow->currentIndex =
            (g_slideshow->currentIndex - 1 + g_slideshow->slides.size())
            % g_slideshow->slides.size();

        UpdateSlide();
    }

    std::shared_ptr<NeoWPF::UI::Grid> CreateMainWindow() {
        g_slideshow = std::make_shared<SlideshowState>();

        auto root = std::make_shared<NeoWPF::UI::Grid>();
        root->SetWidth(600);
        root->SetHeight(500);

        auto mainGrid = std::make_shared<NeoWPF::UI::Grid>();
        mainGrid->AddRow(NeoWPF::UI::SizeMode::Star, 1);
        mainGrid->AddRow(NeoWPF::UI::SizeMode::Auto, 60);
        mainGrid->SetMargin(20);

        auto imageContainer = std::make_shared<NeoWPF::UI::Grid>();
        imageContainer->SetWidthMode(NeoWPF::UI::LayoutSizeMode::Stretch);
        imageContainer->SetHeightMode(NeoWPF::UI::LayoutSizeMode::Stretch);
        imageContainer->SetHorizontalAlignment(
            NeoWPF::UI::HorizontalAlignment::Center
        );
        imageContainer->SetVerticalAlignment(
            NeoWPF::UI::VerticalAlignment::Center
        );

        g_currentImage = std::make_shared<NeoWPF::UI::Image>();
        g_currentImage->SetWidth(400);
        g_currentImage->SetHeight(300);
        g_currentImage->SetHorizontalAlignment(
            NeoWPF::UI::HorizontalAlignment::Center
        );
        g_currentImage->SetVerticalAlignment(
            NeoWPF::UI::VerticalAlignment::Center
        );

        imageContainer->AddChild(g_currentImage);
        mainGrid->AddChild(imageContainer, 0, 0);

        auto controlsGrid = std::make_shared<NeoWPF::UI::Grid>();
        controlsGrid->AddColumn(NeoWPF::UI::SizeMode::Auto, 80);
        controlsGrid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
        controlsGrid->AddColumn(NeoWPF::UI::SizeMode::Auto, 80);
        controlsGrid->SetHorizontalAlignment(
            NeoWPF::UI::HorizontalAlignment::Center
        );
        controlsGrid->SetHorizontalSpacing(15);

        auto backBtn = std::make_shared<NeoWPF::UI::Button>(L"← Back");
        backBtn->SetWidth(80);
        backBtn->SetHeight(35);
        backBtn->Click += []() { OnBackClick(); };
        controlsGrid->AddChild(backBtn, 0, 0);

        g_indexText = std::make_shared<NeoWPF::UI::TextBlock>(L"1 / 4");
        g_indexText->SetHorizontalAlignment(
            NeoWPF::UI::HorizontalAlignment::Center
        );
        g_indexText->SetVerticalAlignment(
            NeoWPF::UI::VerticalAlignment::Center
        );
        g_indexText->SetMargin(10);
        controlsGrid->AddChild(g_indexText, 0, 1);

        auto nextBtn = std::make_shared<NeoWPF::UI::Button>(L"Next →");
        nextBtn->SetWidth(80);
        nextBtn->SetHeight(35);
        nextBtn->Click += []() { OnNextClick(); };
        controlsGrid->AddChild(nextBtn, 0, 2);

        mainGrid->AddChild(controlsGrid, 1, 0);
        root->AddChild(mainGrid);

        UpdateSlide();

        return root;
    }
}
'''

resources_json_content = '''{
  "resources": [
    { "key": "angel", "type": "Image", "path": "angel.jpg", "embed": true },
    { "key": "dog", "type": "Image", "path": "dog.jpg", "embed": false },
    { "key": "gohome", "type": "Image", "path": "gohome.jpg", "embed": false },
    { "key": "soft", "type": "Image", "path": "Soft.jpg", "embed": true }
  ]
}
'''

print("Writing MainWindow.cpp...")
main_window_cpp.parent.mkdir(parents=True, exist_ok=True)
main_window_cpp.write_text(main_window_code, encoding="utf-8")

print("Copying image assets...")
assets_dir.mkdir(parents=True, exist_ok=True)

for filename in ["angel.jpg", "gohome.jpg", "dog.jpg", "Soft.jpg"]:
    source = downloads / filename
    destination = assets_dir / filename

    if not source.exists():
        raise FileNotFoundError(f"Missing asset: {source}")

    shutil.copy2(source, destination)
    print(f"Copied {filename}")

print("Writing resources.json...")
resources_json.parent.mkdir(parents=True, exist_ok=True)
resources_json.write_text(resources_json_content, encoding="utf-8")

print("Done.")