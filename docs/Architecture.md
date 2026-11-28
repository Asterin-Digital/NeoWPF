## Structuring

NeoWPF doesn't have strict architectural rules, but we do expect you to follow the conventions and standards found within the codebase.

NeoWPF follows a simple set of rules when creating a new file (where to place it, how to name it, and what it should do).

You just have to ask yourself these questions to know what to do:

- **What does it do?** This determines its placement and the module's responsibility, ensuring that unrelated code isn't mixed together.
- **What should it be named?** Choose a suitable name based on what the file does. For example, if it handles storage for NeoWPF applications, you can name it `ApplicationStorage.cpp`, located at `Core/Application/ApplicationStorage.cpp`.
- **What category does it belong to?** Look at existing files and determine whether there's already a suitable category. If you can't find one, create a new folder for it (remember not to make the folder name identical to the filename, or vice versa). If you genuinely cannot categorize it well enough, place it inside `Etc/` after review.

When creating a new folder, think about what it represents:

- Is it a system?
- Is it a subsystem within a system?
- Is it a detail within a subsystem?

This helps you determine how to structure new systems and organize existing systems that aren't fully developed.

Always prefer one-word folder names to keep them readable at a glance. For filenames, prefer a maximum of two words. Three words are generally too many and make filenames harder to read.

For example, consider the `Core/` top-level folder. It contains:

- `Application/`
- `Events/`
- `Threading/`

Each is a subsystem of Core, representing an important part of NeoWPF.

Each subsystem contains its own details. For example, `Application/` contains `Startup/`, `Resources/`, `Instance/`, etc.

## Classes

Not every new `.cpp` file needs a matching `.h` file. For example, `Application.h` has several source files implementing the functions exposed to consumers.

That doesn't mean you should create massive header files. Keep their scope well-defined and think carefully before adding more functions to an already large header.

For example, a control should have only one `.h` file. This makes it easier to find and maintain instead of splitting it into multiple micro-header files.

## Source Layout

NeoWPF does not have internal header files in the traditional sense. We have no `.h` files inside `src/` and no `.cpp` files inside `include/`.

We want every API to be accessible and reviewable by users, including those who want clear access to the internals. This does not mean you need to maintain compatibility with internals. You are free to change them as you see fit, including their behavior and structure.

## End

Remember to read the other documentation files. You should rely on all `.md` files inside `docs/` to understand how NeoWPF works and how you are expected to work on and maintain it.

Only a clown follows every rule 1:1 without trying to improve it or otherwise think differently.