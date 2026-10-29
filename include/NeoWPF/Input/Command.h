/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

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
    /*
        @summary
        Represents an executable command.
    */
    class ICommand
    {
    public:
        virtual ~ICommand() = default;

        /*
            @summary
            Reports whether the command accepts the supplied parameter.

            @param parameter
            Value passed to the command.
        */
        virtual bool CanExecute(const std::any& parameter = {}) const = 0;
        /*
            @summary
            Executes the command with the supplied parameter.

            @param parameter
            Value passed to the command.
        */
        virtual void Execute(const std::any& parameter = {}) = 0;

        /*
            @summary
            Registers a callback for command availability changes.

            @param handler
            Callback to register.
        */
        virtual void AddCanExecuteChangedHandler(const std::function<void()>& handler) = 0;
        /*
            @summary
            Unregisters a command availability callback.

            @param handler
            Callback to remove.
        */
        virtual void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) = 0;
        /*
            @summary
            Notifies registered callbacks that command availability may have changed.
        */
        virtual void RaiseCanExecuteChanged() = 0;
    };

    /*
        @summary
        Provides a command and its target to a control.
    */
    class ICommandSource
    {
    public:
        virtual ~ICommandSource() = default;

        /*
            @summary
            Returns the command associated with this source.
        */
        virtual std::shared_ptr<ICommand> GetCommand() const = 0;
        /*
            @summary
            Assigns the command associated with this source.

            @param command
            Command to assign.
        */
        virtual void SetCommand(std::shared_ptr<ICommand> command) = 0;

        /*
            @summary
            Returns the parameter supplied when the command executes.
        */
        virtual std::any GetCommandParameter() const = 0;
        /*
            @summary
            Assigns the parameter supplied when the command executes.

            @param parameter
            Command parameter to assign.
        */
        virtual void SetCommandParameter(const std::any& parameter) = 0;

        /*
            @summary
            Returns the element targeted by the command.
        */
        virtual NeoWPF::UI::UIElement* GetCommandTarget() const = 0;
        /*
            @summary
            Assigns the element targeted by the command.

            @param target
            Target element.
        */
        virtual void SetCommandTarget(NeoWPF::UI::UIElement* target) = 0;
    };

    /*
        @summary
        Implements a command from callback functions.
    */
    class DelegateCommand : public ICommand
    {
    public:
        /*
            @summary
            Callback used to execute a delegate command.
        */
        using ExecuteHandler = std::function<void(const std::any&)>;
        /*
            @summary
            Callback used to determine whether a delegate command can execute.
        */
        using CanExecuteHandler = std::function<bool(const std::any&)>;

        /*
            @summary
            Creates a callback-based command.

            @param executeHandler
            Command execution callback.

            @param canExecuteHandler
            Optional availability callback.
        */
        explicit DelegateCommand(ExecuteHandler executeHandler,
                                 CanExecuteHandler canExecuteHandler = {});

        /*
            @summary
            Evaluates the availability callback for the parameter.
        */
        bool CanExecute(const std::any& parameter = {}) const override;
        /*
            @summary
            Invokes the command execution callback.
        */
        void Execute(const std::any& parameter = {}) override;

        /*
            @summary
            Registers a callback for command availability changes.
        */
        void AddCanExecuteChangedHandler(const std::function<void()>& handler) override;
        /*
            @summary
            Unregisters a command availability callback.
        */
        void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) override;
        /*
            @summary
            Notifies registered callbacks that command availability may have changed.
        */
        void RaiseCanExecuteChanged() override;

    private:
        ExecuteHandler m_executeHandler;
        CanExecuteHandler m_canExecuteHandler;
        std::vector<std::function<void()>> m_canExecuteChangedHandlers;
    };

    /*
        @summary
        Binds command behavior to a UI element.
    */
    class CommandBinding
    {
    public:
        /*
            @summary
            Callback invoked when a command binding executes.
        */
        using ExecuteHandler = std::function<void(NeoWPF::UI::UIElement*, const std::any&)>;
        /*
            @summary
            Callback that determines whether a command binding can execute.
        */
        using CanExecuteHandler = std::function<bool(NeoWPF::UI::UIElement*, const std::any&)>;

        CommandBinding(NeoWPF::UI::UIElement* target = nullptr,
                       ExecuteHandler executeHandler = {},
                       CanExecuteHandler canExecuteHandler = {});

        /*
            @summary
            Returns the element associated with this binding.
        */
        NeoWPF::UI::UIElement* GetTarget() const noexcept;

        /*
            @summary
            Evaluates the binding's availability callback.

            @param source
            Element requesting command execution.

            @param parameter
            Command parameter.
        */
        bool CanExecute(NeoWPF::UI::UIElement* source, const std::any& parameter = {}) const;
        /*
            @summary
            Invokes the binding's execution callback.

            @param source
            Element requesting command execution.

            @param parameter
            Command parameter.
        */
        void Execute(NeoWPF::UI::UIElement* source, const std::any& parameter = {}) const;

    private:
        NeoWPF::UI::UIElement* m_target = nullptr;
        ExecuteHandler m_executeHandler;
        CanExecuteHandler m_canExecuteHandler;
    };

    /*
        @summary
        Routes command execution to the nearest matching binding.
    */
    class RoutedCommand : public ICommand
    {
    public:
        explicit RoutedCommand(std::string name = "");

        /*
            @summary
            Returns the command's registered name.
        */
        const std::string& GetName() const noexcept;

        /*
            @summary
            Adds a command binding for a specific target element.

            @param target
            Element associated with the binding.

            @param executeHandler
            Callback invoked by the binding.

            @param canExecuteHandler
            Optional callback that determines command availability.
        */
        void AddBinding(NeoWPF::UI::UIElement* target,
                        CommandBinding::ExecuteHandler executeHandler,
                        CommandBinding::CanExecuteHandler canExecuteHandler = {});

        /*
            @summary
            Adds a command binding without a specific target element.

            @param executeHandler
            Callback invoked by the binding.

            @param canExecuteHandler
            Optional callback that determines command availability.
        */
        void AddBinding(CommandBinding::ExecuteHandler executeHandler,
                        CommandBinding::CanExecuteHandler canExecuteHandler = {});

        /*
            @summary
            Returns the bindings registered with this command.
        */
        const std::vector<CommandBinding>& GetBindings() const noexcept;

        /*
            @summary
            Checks command availability using the current routing context.
        */
        bool CanExecute(const std::any& parameter = {}) const override;
        /*
            @summary
            Checks command availability for a specified source element.

            @param parameter
            Command parameter.

            @param source
            Element from which command routing begins.
        */
        bool CanExecute(const std::any& parameter, NeoWPF::UI::UIElement* source) const;

        /*
            @summary
            Executes the command using the current routing context.
        */
        void Execute(const std::any& parameter = {}) override;
        /*
            @summary
            Executes the command from a specified source element.

            @param parameter
            Command parameter.

            @param source
            Element from which command routing begins.
        */
        void Execute(const std::any& parameter, NeoWPF::UI::UIElement* source);

        /*
            @summary
            Registers a callback for command availability changes.
        */
        void AddCanExecuteChangedHandler(const std::function<void()>& handler) override;
        /*
            @summary
            Unregisters a command availability callback.
        */
        void RemoveCanExecuteChangedHandler(const std::function<void()>& handler) override;
        /*
            @summary
            Notifies registered callbacks that command availability may have changed.
        */
        void RaiseCanExecuteChanged() override;

    private:
        const CommandBinding* FindNearestBinding(NeoWPF::UI::UIElement* source) const;

        std::string m_name;
        std::vector<CommandBinding> m_bindings;
        std::vector<std::function<void()>> m_canExecuteChangedHandlers;
    };
}
