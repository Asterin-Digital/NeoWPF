# Development guidelines

## Dependency-property integration preferences

When working on dependency-property integration in NeoWPF, prefer the following approach:

- Keep changes narrow and focused on one concern at a time.
- Preserve the existing public API when it remains idiomatic and sufficient.
- Prefer local implementation helpers in the .cpp file when the helper is only used there.
- Avoid adding new helper methods or internal scaffolding unless the design clearly benefits from them.
- Prefer private helpers in the class only when the logic needs access to object state or is part of the class's behavior.
- Avoid creating extra internal headers unless the helper is genuinely shared across multiple classes.

## General editing guidance

- Prefer minimal, conservative changes over broad refactors.
- Explain the reason for non-obvious design choices before introducing them.
- Keep the implementation aligned with the existing architecture rather than introducing parallel patterns.
