1. ## Inheritance Tree

Controls usually follow this inheritance tree (depending on whether they also have a base class or not):
1. Visual (the base class of everything)
2. UIElement
3. FrameworkElement
We will call this Base Inheritance.

The inheritance tree stops here for simple controls like TextBlock, while more complex controls that need hosting and other functionality have a deeper one. For example, Button:
1. Base Inheritance
2. ContentControl
3. ButtonBase (controls that need button-like functionality)

Controls may also inherit other things on the side, like IContentAware (TextBlock.h) or ICommandSource (ButtonBase.h). When creating a new control, it must follow this inheritance tree and not invent a new one from scratch.

A few things to keep in mind:
Visual is not allowed to use or know about the deeper tree (UIElement, FrameworkElement, etc.). This applies to UIElement and FrameworkElement as well. This is done to keep things clean and predictable, and to also not blow up compile times.

2. ## Creating A New Control

It can be a layout or UI control:
- Grid (layout control)
- Button (UI control)

A layout control does not need *Styles.xaml or *Template.cpp files, along with other visual-related files that a control normally needs. Though both layout and UI controls have one shared requirement:
It must have a *Properties.cpp file, to ensure it works in XAML and for many other things as well.

When creating a UI control, you will usually need these files:
- *Styles.xaml
- *Template.cpp (if your control needs one - for example, TextBlock does not)
- *Properties.cpp
- The main control file (where rendering happens)

A UI control typically only needs one header file. You should follow this pattern strictly unless you really need two or more.

Rendering only happens inside the main file, not another one; this is to keep things predictable. Every control (TextBlock, TextBox, Button, etc.) keeps its rendering (BuildRenderCommands) inside the main file, so you need to do that too unless you have a very good reason not to.

The control must be compatible with XAML and use Dependency Properties for various things, to ensure that both XAML and direct C++ usage behave identically, not differently.

If your control needs another control to work (for example, TextBox using ScrollViewer), use it instead of reimplementing it from scratch.

The API surface for your new control must match the general pattern instead of reinventing another one - not doing so causes annoying inconsistencies.

For example, Set/Get are very common; use those for setters or getters instead of plain names, as this keeps the API surface similar between other controls and pleasant to use. But if a method in the API isn't a setter nor a getter, you should not force it into that naming - use the plain name instead.

3. ## Creating A Big New Control

You should not make the full control for the first base implementation. In NeoWPF, we want a first good base implementation that's stable, and well architected so it can later be extended without any problems or huge rewrites.

4. ## Code Cleanliness

You're not required to have god-level coding skills, but it's expected that you at least write decent and maintainable code. For example, with if chains, you're not immediately forced to clean that up into a switch, a method, or whatever your operation requires, but it is recommended that if a developer will later benefit from that refactor, you should clean it up.

Do not duplicate code. If you need utilities, use the existing ones. If there isn't a utility you need, create it instead of hardcoding it in a random file. The rest of the codebase would benefit from that instead of only one file getting it. One common place where utilities live is inside PlatformUtils.cpp. In there you can implement or reuse utilities, and ensure they work correctly across all platforms NeoWPF supports.

Keep files lean and modularize your code. It's not necessary to create a new module out of a file if it's small - it would only annoy the future developer working on that part if they have to browse many other files just to understand one thing. A monolithic file should get split, while a small but focused one shouldn't, unless you expect it to grow a lot.

### Project Structure

Use this structure:

```text
[system]/[subsystem]/[detail]
```

A system cannot contain another system. A subsystem can contain detail only. A detail cannot contain a system or subsystem. Use the same structure across headers, sources, tests, and managed code.

### Dependency Properties

If it's worth it, make sure a property is backed up by dependency property instead of one-off private member; so the rest of NeoWPF can also benefit.

### Platform Separation

Portable code must not contain platform `#if`/`#ifdef` branches or platform headers. Put platform implementations in the corresponding directories:

```text
include/NeoWPF/Platform/Windows/
include/NeoWPF/Platform/Linux/
src/Platform/Windows/
src/Platform/Linux/
```

Use platform-neutral interfaces in shared code and select implementations through CMake.

5. ## Documentation

Do not document source files or internal details. If you need documentation, do it inside the header file for your source file. Follow CommentFormatting.md and CommentingRules.md for more information. In source files, only notes and warnings are allowed - like one simple note telling a developer not to do something, or after you fix a bug, you can leave a note there for the future developer working on that part to know what not to do.

6. ## Stable Header Files

Keep Visual.h, UIElement.h, and FrameworkElement.h stable. Do not constantly modify them unless you have a good reason to do so.

7. ## Legacy Code

NeoWPF is still young; we recommend not leaving any legacy code or preserving an old way of doing something unless lots of things use that API. That also depends - if it's internal code using that API, do not preserve it; simply do a refactor. If it's user code instead, you should gradually deprecate that API and in the end remove it.

8. ## NeoWPF Dependencies

Do not add huge libraries to NeoWPF unless we really need them, but even then, if it's not difficult to make, don't bring that in and instead make our own version of it.