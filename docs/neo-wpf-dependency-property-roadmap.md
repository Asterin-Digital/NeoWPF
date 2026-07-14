## Phase 1 — Core infrastructure

- [x] Define `DependencyProperty` as the unique property identifier type.
- Define `PropertyMetadata` with:
  - [x] default value
  - [x] property changed callback
  - [x] coercion callback
  - [ ] metadata flags
- [x] Define `DependencyPropertyChangedEventArgs` and callback delegate types.
- [x] Implement `DependencyObject` with:
  - [x] `SetValue`
  - [x] `GetValue`
  - [x] `ClearValue`
  - [x] `ReadLocalValue`
  - [x] `GetValueSource`
- [x] Use internal value storage keyed by `DependencyProperty*`.
- [x] Implement type-safe helpers:
  - [x] `GetValue<T>`
  - [x] `SetValue<T>`
- [x] Add runtime type validation for registered property types.
- [x] Support initial value precedence:
  - [x] local values
  - [x] style values
  - [x] default values

## Phase 2 — UI framework integration

- [x] Add `DependencyObject` support to `UIElement` and `FrameworkElement`.
- [ ] Replace direct property fields and legacy getter/setter storage with DP-backed accessors for framework properties.
- [ ] Hook property change notifications into invalidation and layout logic.
- [ ] Add metadata flags for:
  - `AffectsMeasure`
  - `AffectsArrange`
  - `AffectsRender`
  - `Inherits`
- Add dependency-property registration for core framework properties such as:
  - [x] `Visibility`
  - [x] `Opacity`
  - [x] `IsEnabled`
  - [ ] `Width`
  - [ ] `Height`
  - [ ] `HorizontalAlignment`
  - [ ] `VerticalAlignment`
  - [ ] `CornerRadius`
  - [ ] other layout/render related properties

## Phase 3 — Control registration and public APIs

- [ ] Use a WPF-style registration pattern:
  - each control exposes `static DependencyProperty FooProperty`
  - wrapper accessors `GetFoo()` / `SetFoo(...)`
- [x] Register each property using a fully type-safe API, for example:
  - `DependencyProperty::Register<OwnerType, PropertyType>("Foo", metadata)`
- [ ] Implement typed property changed callbacks that call instance methods like:
  - `OnTextChanged`
  - `OnBackgroundColorChanged`
  - `OnPaddingChanged`
- [ ] Convert common control properties first:
  - `Text`
  - `TextColor`
  - `FontSize`
  - `Padding`
  - `BackgroundColor`
  - `BorderColor`
  - `CaretColor`

## Phase 4 — Style and property precedence

- [ ] Implement style property support through dependency-property precedence instead of legacy style methods.
- [ ] Replace styling code paths that use string property names and raw `std::any` values.
- [ ] Use dependency-property lookups and strongly typed setters/getters.
- [x] Support style values as a secondary value source behind local values.
- [ ] Add `PropertyMetadata::Inherits` for inheritable property behavior if needed.
- [x] Defer attached property support to a later phase.

## Phase 5 — Cleanup, stabilization, and docs

- [ ] Remove stale legacy property APIs and old registry code paths entirely.
- [ ] Document the new dependency-property architecture and usage patterns.
- [ ] Add unit tests for:
  - property registration
  - local value resolution
  - style precedence
  - callback dispatch
  - coercion behavior
  - invalidation behavior
- [ ] Add sample usage docs showing dependency-property usage such as:
  - `Button.Text`
  - `TextBox.Padding`
  - `Border.BackgroundColor`

## Relevant files

- `include/NeoWPF/UI/Properties/DependencyProperty.h`
- `include/NeoWPF/UI/Properties/DependencyObject.h`
- `src/UI/Properties/DependencyObject.cpp`
- `include/NeoWPF/UI/Core/Element/UIElement.h`
- `src/UI/Core/Element/UIElement.cpp`
- `include/NeoWPF/UI/Core/Element/FrameworkElement.h`
- `src/UI/Core/Element/FrameworkElement.cpp`
- `src/UI/Controls/*/*Properties.cpp`
