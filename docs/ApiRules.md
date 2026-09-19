# NeoWPF API Rules

These are the naming and API rules used across NeoWPF.

They apply to public headers under `include/NeoWPF` and APIs used by applications, custom controls, XAML, and extensions.

The filename is intentionally `ApiRules.md`.

## Core Rules

### 1. Names should describe what they do

Types use `PascalCase`.

Methods use `PascalCase`.

Parameters and locals use `camelCase`.

Use normal, descriptive names. Don't put implementation details like `Impl`, `Internal`, `Raw`, or `Data` into public names unless they're actually part of the API.

Don't abbreviate names unless they're established terms like `Xaml`, `Uri`, `ARGB`, or `UI`.

### 2. Use `Get` and `Set` for properties

```cpp
Color GetBackgroundColor() const noexcept;
void SetBackgroundColor(Color color) noexcept;
```

Boolean properties use predicates:

```cpp
bool IsEnabled() const noexcept;
bool CanFocus() const noexcept;
bool HasItems() const noexcept;
```

Actions use the action itself:

```cpp
void ClearItems();
void AddItem(std::shared_ptr<FrameworkElement> item);
bool Focus() noexcept;
```

Don't create multiple accessors for the same property.

`GetValue()` is fine when accessing a value through a property descriptor. It shouldn't replace normal property names.

### 3. Return types should make ownership clear

Use values for cheap value types.

Use `const&` when the referenced object has a stable lifetime.

Use `std::optional` or nullable pointers when something can be absent.

Use `std::shared_ptr` only for shared ownership. Use `std::unique_ptr` for exclusive ownership and raw pointers for non-owning links.

`const` methods shouldn't change observable state.

Use `[[nodiscard]]` when ignoring the result is usually a bug.

Only use `noexcept` when the operation actually guarantees it.

### 4. Keep one source of truth

A dependency property is the source of truth for a dependency-property-backed property.

For example, `SetText()` and `GetText()` must stay in sync with `TextProperty()`.

Don't keep duplicate state for the same thing.

Binding, localization, animation, and styles should use the dependency-property system instead of creating parallel state.

Convenience APIs can forward to the canonical API, but shouldn't have different behavior.

### 5. Keep framework boundaries consistent

Follow the established:

`Visual -> UIElement -> FrameworkElement`

inheritance structure.

`On...` methods are framework hooks, not normal application methods.

`Measure`, `Arrange`, and `BuildRenderCommands` are framework pipeline operations.

Framework-only methods should stay protected/internal where possible.

Don't make a control depend on a deeper control layer just because it's convenient. Use composition when that's the established pattern.

## Naming Patterns

| Concept               | Use                                                           |
| --------------------- | ------------------------------------------------------------- |
| Read property         | `GetName()`                                                   |
| Write property        | `SetName(value)`                                              |
| Boolean               | `IsName()`, `HasName()`, `CanName()`                          |
| Add/remove            | `AddItem()`, `RemoveItem()`                                   |
| Remove all            | `ClearItems()`                                                |
| Lookup                | `FindName()`, `TryGetName()`                                  |
| Registration          | `RegisterName()`                                              |
| Event subscription    | `AddNameHandler()`, `RemoveNameHandler()`                     |
| Lifecycle hook        | `OnName()`                                                    |
| Conversion            | `Convert...()`, `TryConvert...()`                             |
| Builder configuration | `WithName(value)`                                             |
| Property handle       | `NameProperty()`                                              |
| Attached property     | `GetName(target)`, `SetName(target, value)`, `NameProperty()` |

`Try...` means failure is expected and non-exceptional.

`With...` is for builders/configuration. Don't use it for mutating a live control.

## Properties

A normal public property should have:

1. `GetName()` / `SetName()`
2. `NameProperty()`
3. XAML metadata/conversion
4. Change notification and any required invalidation

Keep the same property name across all of these.

Use `RegisterAttached` only for properties that are intentionally set on another target type.

Keep defaults, converters, callbacks, and property handles consistent.

Use `double` for logical layout geometry. Use integers for things like indices, counts, spans, and other discrete values.

Don't add `Get...Value()` just to expose backing storage.

## Events and Commands

Events, commands, and property notifications are different things.

Events report something that happened:

```cpp
Click
Loaded
SelectionChanged
```

Commands perform an action:

```cpp
CanExecute
Execute
```

Property notifications report changed state.

Every event subscription needs a reliable way to remove it. Don't rely on addresses of temporary or copied function objects.

Use a subscription token/connection when needed.

`Raise...` is framework-owned. Don't expose public raisers unless applications are supposed to raise the event themselves.

## Collections

Collection APIs should work naturally with C++.

Use:

```cpp
size()
empty()
Add()
Remove()
Clear()
```

when exposing a standard-like collection.

Don't expose both `size()` and `GetCount()` without a real reason.

`SetItems()` replaces the whole collection. `AddItem()` adds one item. Keep those behaviors separate.

Use `std::span`, iterators, or another range type for non-owning bulk input when appropriate.

If `GetItems()` exists, it should be clear whether it returns source items, generated containers, or a snapshot.

Don't expose mutable internal storage through misleading accessors.

## Overloads and Defaults

Overload when the overloads are still the same operation.

Don't create overloads just because pointer/reference or string types are slightly different.

Prefer strongly typed APIs over `std::any` or variadic APIs.

Defaults should represent the normal, safe behavior.

Don't use `0`, `nullptr`, or an empty string as a hidden mode switch.

If a lookup can fail, make that clear with something like `TryGet...` or `GetOrCreate...`.

## API Visibility

Think of every public declaration as one of these:

**Application API**
Stable and intended for normal app code.

**Extension API**
Intended for custom controls and framework extensions.

**Framework API**
Used internally by the framework and framework subclasses.

**Implementation API**
Not part of the public contract.

Don't expose implementation APIs just because generated XAML or registration code needs them. Make a small extension boundary instead.


## Current Violations and Required Review

These are existing declarations that conflict with the rules above. They are recorded here to prevent new code from copying them. This document does not silently change them; each should be handled as a deliberate compatibility decision.

| Severity | Existing API | Problem | Preferred direction |
| --- | --- | --- | --- |
| High | `Core::Event::operator-=` in `include/NeoWPF/Core/Events/Event.h` | Removal compares the address of the incoming `std::function`; a copied or temporary equivalent callback cannot be reliably removed. | Return a connection/token from subscription and remove by token, or use a stable handler id. |
| High | `RegisterProperty`, `RegisterAttachedProperty`, and `RegisterStaticPropertyHandle` in `include/NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h` | Several overlapping registration vocabularies expose implementation details and make the canonical registration path unclear. | Keep one internal registration path and one explicit attached-property path; hide static-handle plumbing from application API. |
| High | `SetValue(const std::string&, ...)` and `GetValue<T>(const std::string&)` in `include/NeoWPF/UI/Properties/DependencyObject.h` | String lookup can silently do nothing or return `T{}` for a missing property, hiding registration errors. | Keep descriptor-based access canonical; make name-based access explicit `TryGetValue`/`TrySetValue` or return a diagnostic result. |
| Medium | `UIElement::SetVisibility` and `UIElement::SetVisible` in `include/NeoWPF/UI/Core/Element/UIElement.h` | `SetVisible` is a redundant alias and violates the one canonical accessor rule. | Retain `SetVisibility`; deprecate and remove the alias when compatibility permits. |
| Medium | `ItemsControl::GetItems()` and `GetItemsValue()` in `include/NeoWPF/UI/Controls/ItemsControl/ItemsControl.h` | Two getters expose the same collection under different names without a documented semantic distinction. | Keep `GetItems()` for the public meaning; make any binding adapter internal or name it for its actual role. |
| Medium | `INotifyPropertyChanged::AddPropertyChangedHandler` and `RemovePropertyChangedHandler` in `include/NeoWPF/UI/Data/INotifyPropertyChanged.h` | The pair is token-shaped (`size_t`) but does not establish whether ids are stable, reusable, or thread-safe. | Document token lifetime and invalidation, or return a connection object. |
| Medium | `ICommand::AddCanExecuteChangedHandler` and `RemoveCanExecuteChangedHandler` in `include/NeoWPF/Input/Command.h` | Removal accepts a function object instead of a stable subscription identity and repeats the event-removal problem. | Use a handler token/connection. |
| Low | `PropertyChangedCallback` and `PropertyChangedCallbackFunction` in `include/NeoWPF/UI/Properties/DependencyProperty.h` | The alias adds a `Function` suffix without changing semantics. | Keep the shorter callback type name and avoid copying this alias pattern. |
| Low | `CoerceToWString` in `include/NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h` | A public-looking conversion helper is located in registration plumbing and returns an empty string for unsupported values, conflating failure with a valid result. | Move it to an internal conversion utility or return an explicit conversion result. |
| Low | `DoubleCollection::GetCount()` and `IsEmpty()` in `include/NeoWPF/UI/Core/Collections/DoubleCollection.h` | These are reasonable framework names but diverge from standard container composition (`size`, `empty`). | Keep for compatibility if widely used; use standard names for new collection types. |
| Low | `Binding::WithMode` and `Binding::WithSource` in `include/NeoWPF/UI/Data/Binding.h` | These are appropriate builder names, but the same `With` pattern must not spread to live controls where it would obscure mutation. | Restrict `With...` to immutable/configuration-style construction. |
| Low | `Button::SetFontFamily` in `include/NeoWPF/UI/Controls/Button/Button.h` | The setter silently ignores an empty value, so callers cannot clear the property and the setter is not a total assignment. | Define empty-value semantics explicitly or reject with a result/error policy. |
