# NeoWPF — How to document and how to NOT document headers

## General Rules

* Document public classes, structs, enums, functions, properties, events, and typedefs.
* Keep `@summary` to a single sentence whenever possible.
* Use `@details` only when additional explanation is necessary.
* Omit unused tags.
* Do not document implementation details or algorithms.

## Available Tags

| Tag             | Description                                     |
| --------------- | ----------------------------------------------- |
| `@summary`      | Brief description of the declaration.           |
| `@details`      | Additional information about behavior or usage. |
| `@param <name>` | Describes a parameter. One tag per parameter.   |
| `@returns`      | Describes the return value.                     |
| `@note`         | Helpful information or behavior.                |
| `@warning`      | Important limitations or caveats.               |
| `@example`      | Example usage.                                  |
| `@related`      | Related declarations.                           |
| `@todo`         | Planned improvements or missing functionality.  |
| `@devwarn`      | Developer warning code and description used by diagnostics. |
| `@internal`     | Internal methods that aren't public API's       |

## More Rules

1. You cannot document a constructor nor a destructor.
2. You cannot describe a use case, when to use, or whatnot when documenting something
3. You cannot document static Dependency Properties
4. You cannot include internal arlgorithm details in documentation
5. You MUST, under any circumstances, mark said method as @internal if IT IS internal.
6. todo, note tags cannot be used in documented header files.
7. You cannot use regular // plain comments, or /* plain multiline comments */ in a documented header file
8. Every comment block must have it's tag at the top, and the comment at the bottom, they cannot be one the same line.
9. todo, note tags can only be used in source files when you leave a small small note that tells the future developer what not to do, and a todo when something is unfinished.
10. Leaving a known internal method name not marked as @internal will trigger a compile error at build time.

## Example

```cpp
/*
    @summary
    Measures the element using the available layout space.

    @param availableWidth
    Maximum width available.

    @param availableHeight
    Maximum height available.
*/
virtual void Measure(double availableWidth, double availableHeight);
```
