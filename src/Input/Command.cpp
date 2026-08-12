#include <NeoWPF/Input/Command.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <algorithm>

namespace NeoWPF::Input
{
    DelegateCommand::DelegateCommand(ExecuteHandler executeHandler,
                                     CanExecuteHandler canExecuteHandler)
        : m_executeHandler(std::move(executeHandler)),
          m_canExecuteHandler(std::move(canExecuteHandler))
    {
    }

    bool DelegateCommand::CanExecute(const std::any& parameter) const
    {
        if (!m_canExecuteHandler) {
            return true;
        }
        return m_canExecuteHandler(parameter);
    }

    void DelegateCommand::Execute(const std::any& parameter)
    {
        if (m_executeHandler && CanExecute(parameter)) {
            m_executeHandler(parameter);
        }
    }

    void DelegateCommand::AddCanExecuteChangedHandler(const std::function<void()>& handler)
    {
        if (handler) {
            m_canExecuteChangedHandlers.push_back(handler);
        }
    }

    void DelegateCommand::RemoveCanExecuteChangedHandler(const std::function<void()>& handler)
    {
        auto it = std::remove_if(m_canExecuteChangedHandlers.begin(), m_canExecuteChangedHandlers.end(),
                                 [&handler](const std::function<void()>& current) {
                                     return current.target<void()>() == handler.target<void()>();
                                 });
        if (it != m_canExecuteChangedHandlers.end()) {
            m_canExecuteChangedHandlers.erase(it, m_canExecuteChangedHandlers.end());
        }
    }

    void DelegateCommand::RaiseCanExecuteChanged()
    {
        for (const auto& handler : m_canExecuteChangedHandlers) {
            if (handler) {
                handler();
            }
        }
    }

    CommandBinding::CommandBinding(NeoWPF::UI::UIElement* target,
                                   ExecuteHandler executeHandler,
                                   CanExecuteHandler canExecuteHandler)
        : m_target(target),
          m_executeHandler(std::move(executeHandler)),
          m_canExecuteHandler(std::move(canExecuteHandler))
    {
    }

    NeoWPF::UI::UIElement* CommandBinding::GetTarget() const noexcept
    {
        return m_target;
    }

    bool CommandBinding::CanExecute(NeoWPF::UI::UIElement* source, const std::any& parameter) const
    {
        if (!m_canExecuteHandler) {
            return true;
        }
        return m_canExecuteHandler(source, parameter);
    }

    void CommandBinding::Execute(NeoWPF::UI::UIElement* source, const std::any& parameter) const
    {
        if (m_executeHandler && CanExecute(source, parameter)) {
            m_executeHandler(source, parameter);
        }
    }

    bool RoutedCommand::CanExecute(const std::any& parameter) const
    {
        return CanExecute(parameter, nullptr);
    }

    bool RoutedCommand::CanExecute(const std::any& parameter, NeoWPF::UI::UIElement* source) const
    {
        const auto* binding = FindNearestBinding(source);
        if (!binding) {
            return true;
        }
        return binding->CanExecute(source, parameter);
    }

    void RoutedCommand::Execute(const std::any& parameter)
    {
        Execute(parameter, nullptr);
    }

    void RoutedCommand::Execute(const std::any& parameter, NeoWPF::UI::UIElement* source)
    {
        const auto* binding = FindNearestBinding(source);
        if (!binding) {
            return;
        }
        binding->Execute(source, parameter);
    }

    void RoutedCommand::AddCanExecuteChangedHandler(const std::function<void()>& handler)
    {
        if (handler) {
            m_canExecuteChangedHandlers.push_back(handler);
        }
    }

    void RoutedCommand::RemoveCanExecuteChangedHandler(const std::function<void()>& handler)
    {
        auto it = std::remove_if(m_canExecuteChangedHandlers.begin(), m_canExecuteChangedHandlers.end(),
                                 [&handler](const std::function<void()>& current) {
                                     return current.target<void()>() == handler.target<void()>();
                                 });
        if (it != m_canExecuteChangedHandlers.end()) {
            m_canExecuteChangedHandlers.erase(it, m_canExecuteChangedHandlers.end());
        }
    }

    void RoutedCommand::RaiseCanExecuteChanged()
    {
        for (const auto& handler : m_canExecuteChangedHandlers) {
            if (handler) {
                handler();
            }
        }
    }

    const CommandBinding* RoutedCommand::FindNearestBinding(NeoWPF::UI::UIElement* source) const
    {
        if (!source)
        {
            for (const auto& binding : m_bindings)
            {
                if (!binding.GetTarget()) {
                    return &binding;
                }
            }
            return nullptr;
        }

        for (auto* current = source; current != nullptr; current = dynamic_cast<NeoWPF::UI::UIElement*>(current->GetParent()))
        {
            for (const auto& binding : m_bindings)
            {
                if (binding.GetTarget() == current)
                {
                    return &binding;
                }
            }
        }

        for (const auto& binding : m_bindings)
        {
            if (!binding.GetTarget())
            {
                return &binding;
            }
        }

        return nullptr;
    }

    RoutedCommand::RoutedCommand(std::string name)
        : m_name(std::move(name))
    {
    }

    const std::string& RoutedCommand::GetName() const noexcept
    {
        return m_name;
    }

    void RoutedCommand::AddBinding(NeoWPF::UI::UIElement* target,
                                  CommandBinding::ExecuteHandler executeHandler,
                                  CommandBinding::CanExecuteHandler canExecuteHandler)
    {
        m_bindings.emplace_back(target, std::move(executeHandler), std::move(canExecuteHandler));
    }

    void RoutedCommand::AddBinding(CommandBinding::ExecuteHandler executeHandler,
                                  CommandBinding::CanExecuteHandler canExecuteHandler)
    {
        m_bindings.emplace_back(nullptr, std::move(executeHandler), std::move(canExecuteHandler));
    }

    const std::vector<CommandBinding>& RoutedCommand::GetBindings() const noexcept
    {
        return m_bindings;
    }
}
