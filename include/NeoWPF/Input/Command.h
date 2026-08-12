#pragma once

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NeoWPF::UI
{
    class UIElement;
}

namespace NeoWPF::Input
{
    class ICommand
    {
    public:
        virtual ~ICommand() = default;

        virtual bool CanExecute(const std::any& parameter = {}) const = 0;
        virtual void Execute(const std::any& parameter = {}) = 0;

        virtual void AddCanExecuteChangedHandler(const std::function<void()>& handler) = 0;
        virtual void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) = 0;
        virtual void RaiseCanExecuteChanged() = 0;
    };

    class ICommandSource
    {
    public:
        virtual ~ICommandSource() = default;

        virtual std::shared_ptr<ICommand> GetCommand() const = 0;
        virtual void SetCommand(std::shared_ptr<ICommand> command) = 0;

        virtual std::any GetCommandParameter() const = 0;
        virtual void SetCommandParameter(const std::any& parameter) = 0;

        virtual NeoWPF::UI::UIElement* GetCommandTarget() const = 0;
        virtual void SetCommandTarget(NeoWPF::UI::UIElement* target) = 0;
    };

    class DelegateCommand : public ICommand
    {
    public:
        using ExecuteHandler = std::function<void(const std::any&)>;
        using CanExecuteHandler = std::function<bool(const std::any&)>;

        explicit DelegateCommand(ExecuteHandler executeHandler,
                                 CanExecuteHandler canExecuteHandler = {});

        bool CanExecute(const std::any& parameter = {}) const override;
        void Execute(const std::any& parameter = {}) override;

        void AddCanExecuteChangedHandler(const std::function<void()>& handler) override;
        void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) override;
        void RaiseCanExecuteChanged() override;

    private:
        ExecuteHandler m_executeHandler;
        CanExecuteHandler m_canExecuteHandler;
        std::vector<std::function<void()>> m_canExecuteChangedHandlers;
    };

    class CommandBinding
    {
    public:
        using ExecuteHandler = std::function<void(NeoWPF::UI::UIElement*, const std::any&)>;
        using CanExecuteHandler = std::function<bool(NeoWPF::UI::UIElement*, const std::any&)>;

        CommandBinding(NeoWPF::UI::UIElement* target = nullptr,
                       ExecuteHandler executeHandler = {},
                       CanExecuteHandler canExecuteHandler = {});

        NeoWPF::UI::UIElement* GetTarget() const noexcept;

        bool CanExecute(NeoWPF::UI::UIElement* source, const std::any& parameter = {}) const;
        void Execute(NeoWPF::UI::UIElement* source, const std::any& parameter = {}) const;

    private:
        NeoWPF::UI::UIElement* m_target = nullptr;
        ExecuteHandler m_executeHandler;
        CanExecuteHandler m_canExecuteHandler;
    };

    class RoutedCommand : public ICommand
    {
    public:
        explicit RoutedCommand(std::string name = "");

        const std::string& GetName() const noexcept;

        void AddBinding(NeoWPF::UI::UIElement* target,
                        CommandBinding::ExecuteHandler executeHandler,
                        CommandBinding::CanExecuteHandler canExecuteHandler = {});

        void AddBinding(CommandBinding::ExecuteHandler executeHandler,
                        CommandBinding::CanExecuteHandler canExecuteHandler = {});

        const std::vector<CommandBinding>& GetBindings() const noexcept;

        bool CanExecute(const std::any& parameter = {}) const override;
        bool CanExecute(const std::any& parameter, NeoWPF::UI::UIElement* source) const;

        void Execute(const std::any& parameter = {}) override;
        void Execute(const std::any& parameter, NeoWPF::UI::UIElement* source);

        void AddCanExecuteChangedHandler(const std::function<void()>& handler) override;
        void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) override;
        void RaiseCanExecuteChanged() override;

    private:
        const CommandBinding* FindNearestBinding(NeoWPF::UI::UIElement* source) const;

        std::string m_name;
        std::vector<CommandBinding> m_bindings;
        std::vector<std::function<void()>> m_canExecuteChangedHandlers;
    };
}
