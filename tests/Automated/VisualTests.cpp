#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_set>
#include <vector>

namespace {

class TestVisual : public NeoWPF::UI::Visual {
public:
    TestVisual() = default;

    void SetInitializationCount(int count) { m_initializationCount = count; }
    int GetInitializationCount() const noexcept { return m_initializationCount; }

    void OnInitialize() override
    {
        ++m_initializationCount;
    }

    std::vector<NeoWPF::UI::Visual*> AddedChildren;

    void OnAddChild(NeoWPF::UI::Visual* child) override
    {
        AddedChildren.push_back(child);
    }

private:
    int m_initializationCount = 0;
};

}

TEST(VisualTests, ConstructorRegistersVisualAndGetRegisteredVisualsTracksIt)
{
    NeoWPF::Core::Application::Initialize();

    const auto registeredBefore = NeoWPF::UI::Visual::GetRegisteredVisuals();
    auto visual = std::make_shared<TestVisual>();
    const auto registeredAfter = NeoWPF::UI::Visual::GetRegisteredVisuals();

    EXPECT_NE(std::find(registeredAfter.begin(), registeredAfter.end(), visual.get()), registeredAfter.end());
    EXPECT_EQ(registeredAfter.size(), registeredBefore.size() + 1u);

    visual.reset();

    const auto registeredFinal = NeoWPF::UI::Visual::GetRegisteredVisuals();
    EXPECT_EQ(registeredFinal.size(), registeredBefore.size());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, AddVisualChildTracksParentAndRegistersChildren)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    const auto child = std::make_shared<TestVisual>();

    parent->AddVisualChild(child);

    ASSERT_EQ(parent->GetVisualChildren().size(), 1u);
    EXPECT_EQ(parent->GetVisualChildren()[0], child.get());
    EXPECT_EQ(child->GetParent(), parent.get());
    EXPECT_EQ(parent->GetLogicalChildren().size(), 0u);

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, ReMarkingTreeDirtyChildPropagatesToUninitializedParent)
{
    NeoWPF::Core::Application::Initialize();

    auto parent = std::make_shared<TestVisual>();
    auto child = std::make_shared<TestVisual>();
    child->MarkTreeDirty();

    parent->AddVisualChild(child);
    EXPECT_FALSE(parent->IsTreeDirty());
    EXPECT_FALSE(parent->IsLayoutDirty());

    child->MarkTreeDirty();

    EXPECT_TRUE(parent->IsTreeDirty());
    EXPECT_FALSE(parent->IsLayoutDirty());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, ReMarkingLayoutDirtyChildPropagatesToUninitializedParent)
{
    NeoWPF::Core::Application::Initialize();

    auto parent = std::make_shared<TestVisual>();
    auto child = std::make_shared<TestVisual>();
    child->MarkLayoutDirty();

    parent->AddVisualChild(child);
    EXPECT_FALSE(parent->IsTreeDirty());
    EXPECT_FALSE(parent->IsLayoutDirty());

    child->MarkLayoutDirty();

    EXPECT_TRUE(parent->IsTreeDirty());
    EXPECT_TRUE(parent->IsLayoutDirty());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, AddChildAddsLogicalChildAndInvokesOnAddChild)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    const auto child = std::make_shared<TestVisual>();

    parent->AddChild(child);

    ASSERT_EQ(parent->GetVisualChildren().size(), 1u);
    ASSERT_EQ(parent->GetLogicalChildren().size(), 1u);
    EXPECT_EQ(parent->GetLogicalChildren()[0], child.get());
    EXPECT_EQ(parent->AddedChildren.size(), 1u);
    EXPECT_EQ(parent->AddedChildren[0], child.get());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, AllowsWorkerThreadStructuralVisualMutation)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    const auto child = std::make_shared<TestVisual>();
    std::thread worker([&parent, &child] {
        parent->AddVisualChild(child);
    });
    worker.join();

    ASSERT_EQ(parent->GetVisualChildren().size(), 1u);
    EXPECT_EQ(parent->GetVisualChildren()[0], child.get());
    EXPECT_FALSE(parent->IsLayoutDirty());
    EXPECT_TRUE(NeoWPF::Core::Threading::ThreadAffinity::IsMainThread());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, WorkerCanMutateVisualTreeEvery50Milliseconds)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    std::vector<std::shared_ptr<TestVisual>> children;
    for (int index = 0; index < 8; ++index) {
        children.push_back(std::make_shared<TestVisual>());
    }

    std::atomic<bool> running{true};
    std::atomic<bool> invalidSnapshot{false};
    std::thread worker([&] {
        for (int cycle = 0; cycle < 20; ++cycle) {
            for (const auto& child : children) {
                parent->AddVisualChild(child);
            }
            for (const auto& child : children) {
                parent->RemoveVisualChild(child);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        running = false;
    });

    while (running.load()) {
        const auto snapshot = parent->GetVisualChildren();
        std::unordered_set<NeoWPF::UI::Visual*> uniqueChildren;
        for (auto* child : snapshot) {
            if (!child || !uniqueChildren.insert(child).second) {
                invalidSnapshot = true;
                break;
            }
        }
    }
    worker.join();

    EXPECT_FALSE(invalidSnapshot.load());
    EXPECT_TRUE(parent->GetVisualChildren().empty());
    EXPECT_FALSE(parent->IsLayoutDirty());
    EXPECT_FALSE(parent->IsTreeDirty());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, RemoveVisualChildAndRemoveFromParentClearTreeLinks)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    const auto child = std::make_shared<TestVisual>();
    parent->AddVisualChild(child);

    parent->RemoveVisualChild(child);
    EXPECT_TRUE(parent->GetVisualChildren().empty());
    EXPECT_EQ(child->GetParent(), nullptr);

    const auto secondParent = std::make_shared<TestVisual>();
    const auto secondChild = std::make_shared<TestVisual>();
    secondParent->AddChild(secondChild);
    secondChild->RemoveFromParent();

    EXPECT_TRUE(secondParent->GetVisualChildren().empty());
    EXPECT_TRUE(secondParent->GetLogicalChildren().empty());
    EXPECT_EQ(secondChild->GetParent(), nullptr);

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, InitializationRecursesAndOnlyRunsOnce)
{
    NeoWPF::Core::Application::Initialize();

    auto parent = std::make_shared<TestVisual>();
    auto child = std::make_shared<TestVisual>();
    auto grandChild = std::make_shared<TestVisual>();

    parent->AddVisualChild(child);
    child->AddVisualChild(grandChild);

    EXPECT_FALSE(parent->IsInitialized());
    EXPECT_FALSE(child->IsInitialized());
    EXPECT_FALSE(grandChild->IsInitialized());

    parent->Initialize();

    EXPECT_TRUE(parent->IsInitialized());
    EXPECT_TRUE(child->IsInitialized());
    EXPECT_TRUE(grandChild->IsInitialized());
    EXPECT_EQ(parent->GetInitializationCount(), 1);
    EXPECT_EQ(child->GetInitializationCount(), 1);
    EXPECT_EQ(grandChild->GetInitializationCount(), 1);

    parent->Initialize();
    EXPECT_EQ(parent->GetInitializationCount(), 1);

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, AddVisualChildWhileInitializedInvokesInitializeOnNewChild)
{
    NeoWPF::Core::Application::Initialize();

    auto parent = std::make_shared<TestVisual>();
    parent->Initialize();

    auto child = std::make_shared<TestVisual>();
    parent->AddVisualChild(child);

    EXPECT_TRUE(child->IsInitialized());
    EXPECT_EQ(child->GetInitializationCount(), 1);

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, GetVisualDescendantsAndZOrderTrackTreeShape)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = std::make_shared<TestVisual>();
    const auto childA = std::make_shared<TestVisual>();
    const auto childB = std::make_shared<TestVisual>();
    const auto grandChild = std::make_shared<TestVisual>();

    childA->SetZIndex(2);
    childB->SetZIndex(1);

    root->AddVisualChild(childA);
    root->AddVisualChild(childB);
    childA->AddVisualChild(grandChild);

    const auto descendants = root->GetVisualDescendants();
    ASSERT_EQ(descendants.size(), 3u);
    EXPECT_EQ(descendants[0], childA.get());
    EXPECT_EQ(descendants[1], grandChild.get());
    EXPECT_EQ(descendants[2], childB.get());

    const auto zOrdered = root->GetVisualChildrenInZOrder();
    ASSERT_EQ(zOrdered.size(), 2u);
    EXPECT_EQ(zOrdered[0], childB.get());
    EXPECT_EQ(zOrdered[1], childA.get());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, DirtyFlagsPropagateAndCanBeCleared)
{
    NeoWPF::Core::Application::Initialize();

    auto parent = std::make_shared<TestVisual>();
    auto child = std::make_shared<TestVisual>();
    parent->AddVisualChild(child);

    EXPECT_FALSE(parent->IsTreeDirty());
    EXPECT_FALSE(parent->IsLayoutDirty());
    EXPECT_FALSE(child->IsTreeDirty());
    EXPECT_FALSE(child->IsLayoutDirty());

    child->MarkLayoutDirty();
    EXPECT_TRUE(child->IsLayoutDirty());
    EXPECT_TRUE(parent->IsTreeDirty());
    EXPECT_TRUE(parent->IsLayoutDirty());

    parent->ClearTreeDirty();
    parent->ClearLayoutDirty();
    child->ClearTreeDirty();
    child->ClearLayoutDirty();

    EXPECT_FALSE(parent->IsTreeDirty());
    EXPECT_FALSE(parent->IsLayoutDirty());
    EXPECT_FALSE(child->IsTreeDirty());
    EXPECT_FALSE(child->IsLayoutDirty());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, FindInVisualTreeReturnsFirstMatchDepthFirst)
{
    NeoWPF::Core::Application::Initialize();

    const auto root = std::make_shared<TestVisual>();
    const auto childA = std::make_shared<TestVisual>();
    const auto childB = std::make_shared<TestVisual>();
    const auto grandChild = std::make_shared<TestVisual>();

    root->AddVisualChild(childA);
    root->AddVisualChild(childB);
    childA->AddVisualChild(grandChild);

    const auto found = root->FindInVisualTree([](NeoWPF::UI::Visual* visual) {
        return visual && visual->GetZIndex() == 42;
    });
    EXPECT_EQ(found, nullptr);

    grandChild->SetZIndex(42);
    const auto foundByIndex = root->FindInVisualTree([](NeoWPF::UI::Visual* visual) {
        return visual && visual->GetZIndex() == 42;
    });
    EXPECT_EQ(foundByIndex, grandChild.get());

    NeoWPF::Core::Application::Shutdown();
}

TEST(VisualTests, AddLogicalChildStoresLogicalChildrenWithoutVisualLinking)
{
    NeoWPF::Core::Application::Initialize();

    const auto parent = std::make_shared<TestVisual>();
    const auto child = std::make_shared<TestVisual>();

    parent->AddLogicalChild(child);

    EXPECT_TRUE(parent->GetLogicalChildren().size() == 1u);
    EXPECT_EQ(parent->GetLogicalChildren()[0], child.get());
    EXPECT_TRUE(parent->GetVisualChildren().empty());
    EXPECT_EQ(child->GetParent(), nullptr);

    NeoWPF::Core::Application::Shutdown();
}
