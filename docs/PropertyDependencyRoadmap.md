## Phase 1 — Core infrastructure

- [x] Define `DependencyProperty` as the unique property identifier type.
- Define `PropertyMetadata` with:
  - [x] default value
  - [x] property changed callback
  - [x] coercion callback
  - [x] metadata flags
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
- [x] Replace direct property fields and legacy getter/setter storage with DP-backed accessors for the FrameworkElement properties integrated so far.
- [x] Hook property change notifications into invalidation and layout logic.
- [x] Add metadata flags for:
  - `AffectsMeasure`
  - `AffectsArrange`
  - `AffectsRender`
  - `Inherits`
- Add dependency-property registration for core framework properties such as:
  - [x] `Visibility`
  - [x] `Opacity`
  - [x] `IsEnabled`
  - [x] `Width`
  - [x] `Height`
  - [x] `HorizontalAlignment`
  - [x] `VerticalAlignment`
  - [x] `CornerRadius`
  - [x] `WidthMode`
  - [x] `HeightMode`
  - [ ] other layout/render related properties

## Phase 3 — Control registration and public APIs

  - each control exposes `static DependencyProperty FooProperty`
  - wrapper accessors `GetFoo()` / `SetFoo(...)`
- [x] Register each property using a fully type-safe API, for example:
  - `DependencyProperty::Register<OwnerType, PropertyType>("Foo", metadata)`
- [x] Implement typed property changed callbacks that call instance methods like:
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
- [x] Replace styling code paths that use string property names and raw `std::any` values.
- [x] Use dependency-property lookups and strongly typed setters/getters.
- [x] Support style values as a secondary value source behind local values.
- [ ] Add `PropertyMetadata::Inherits` for inheritable property behavior if needed.
- [x] Defer attached property support to a later phase.

## Phase 5 — Cleanup, stabilization
- [ ] Add unit tests for:
  - property registration
  - local value resolution
  - style precedence
  - callback dispatch
  - coercion behavior
  - invalidation behavior
