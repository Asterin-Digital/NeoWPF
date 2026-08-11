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
| `@since`        | Version the API was introduced.                 |
| `@related`      | Related declarations.                           |
| `@todo`         | Planned improvements or missing functionality.  |
| `@devwarn`      | Developer warning code and description used by diagnostics. |
| `@internal`     | Internal methods that aren't public API's       |

## Example

```cpp
/*
    @summary
    Measures the element using the available layout space.

    @param availableWidth
    Maximum width available.

    @param availableHeight
    Maximum height available.

    @devwarn NEO-UI101 Dangling child reference detected.
    This warning is queued when a visual child is attached without active shared ownership.

    @note
    Called before Arrange().
*/
virtual void Measure(int availableWidth, int availableHeight);
```
