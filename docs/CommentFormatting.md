# NeoWPF — How to document and how to NOT document headers

## General Rules

* Document public classes, structs, enums, functions, properties, events, and typedefs.
* Keep `@summary` to a single sentence whenever possible.
* Use `@details` only when additional explanation is necessary.
* Omit unused tags.
* Do not document implementation details or algorithms.
* Avoid documenting internal header files, you can but keep it only for self-notes.

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
| `@since`        | Version the API was introduced.                 |
| `@related`      | Related declarations.                           |
| `@todo`         | Planned improvements or missing functionality.  |

## Example

```cpp
/*
    @summary
    Measures the element using the available layout space.

    @param availableWidth
    Maximum width available.

    @param availableHeight
    Maximum height available.

    @note
    Called before Arrange().
*/
virtual void Measure(int availableWidth, int availableHeight);
```
