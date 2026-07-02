## How to use comments in code

### Where to write comments?

Comments should be primarily written inside header files, and not inside source files (not in .cpp, inside .h/hpp).  
Refrain from writing comments inside source files, you should only do so when the code itself is messy, and really hard to understand.

If a source file needs comments to be understood, consider pulling that messy logic into a separate helper function with a clear name instead.

---

### Should I use splitter comments? (E.g: // ---------------- X SECTION ----------------)

No, you should always refrain from using or adding splitter comments in source or header files. They are not needed at all unless the file itself is really long, and really messy. But even then, you should simply refactor that file.

If you feel like adding a splitter, use that as a cue to split the file into smaller modules instead.

---

### How should I write comments?

Primarily, prefer one-line, and compact comments that simply summarizes what this piece of code does. Refrain from summarizing what that piece of code does if it is already obvious, or if it's short/small.

Use comments to explain what the code does if it's unclear or not obvious from simply reading it, or if it's too long. Don't use them as waypoints or use them in a way to keep track of things, as it's unnecessary and damages the readability of code.

Avoid commenting on plain variable declarations, simple assignments, or basic control flow statements (if, for, while) – unless the condition or the assigned value is non‑obvious or hides a trap.

Do not add step‑by‑step narratives or numbered sequences inside comments (like `// 1. do this, // 2. do that`). Those are waypoints in disguise and add clutter.

Prefer writing comments from third-person and not from first-person.

Prefer mixed tense documentation, past, imperative, etc, do not document solely from present simple.

Do not be overly formal, professional or polite when writing comments, write them in a casual, or colloquial way. Neither should you use trailing comments at the end of a line – they break visual flow and are harder to spot than standalone comments.

Do not add comments inside a file that already has a lot of them, unless absolutely necessary or if the user asked to do so. If it's a heavily documented file, you may proceed in adding more comments to document what any other function or other piece of code does that has not been already documented, but only for public or exported functions – internal helpers rarely need the extra noise.

For sentence structure you may use fragments, abbreviations or keywords only.

Do not start comments with "This function" or "This method" – it's filler that adds nothing. Cut straight to what matters.

If a comment only repeats what the function name already says (e.g., `// validates email` on a function named `validateEmail`), leave it out – it adds zero value.

Write comments for the non‑obvious stuff: edge cases, weird workarounds, performance hacks, bug fixes. That's where comments actually help.

When you change existing code, review and update or delete any comments that refer to the old logic. Stale comments that lie to the reader are worse than having no comment at all.

Refrain from self-signing code, or leaving names inside code.

Refrain from adding comments at the top of the file or bottom of the file using the namespace, or name of the current file.

---

### Should I use special characters?

Refrain from doing so, never use em dashes, or hyphens inside code unless the user directly or specifically asks you to do so. Keep punctuation minimal – a full stop here and there is fine, but nothing fancy.

---

### What documentation structure should I use when I want to document an API?

Refer to CommentFormatting.md for available tags, and what to use. All rules inside here apply.

Only write structured doc tags (`@param`, `@return`, `@throws`) for public APIs that are consumed by other modules or external users. For internal static helpers, a single plain‑English line above the function is enough – don't bloat them with formal boilerplate.


### What example can I base off of to see what's not allowed?

// EmailValidator.h

#ifndef EMAIL_VALIDATOR_H
#define EMAIL_VALIDATOR_H

#include <string>

class EmailValidator
{
public:
    // checks basic structure of email input, keeps it simple and safe
    static bool isValid(std::string const& email);
};

#endif

Refrain from adding the filenames anywhere in the file like this example does "// EmailValidator.h", it's visual clutter.
Refrain from documenting what a function does when it's already obvious by the function name "isValid"
already tells you what it does, adding a comment there is visual clutter.
Do not use uesless # defs unless absolutely needed.