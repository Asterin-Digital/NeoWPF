# Feature Review and Approval Process

Every feature must go through this process before it is approved for implementation.

Every feature must go through every stage, even if it is small. A small feature may need less investigation in some areas, but those areas still need to be reviewed.

## 1. Feature Submission

Describe the feature before jumping into implementation.

The submission must include:

- **Problem:** What problem are we trying to solve?
- **Proposed solution:** What should the feature do?
- **Expected result:** What should the user be able to accomplish?
- **Use cases:** When and where would this feature be useful?
- **Scope:** What is included, and what is explicitly excluded?
- **Affected areas:** Which existing systems, controls, APIs, or modules could be involved?

Describe the problem separately from the proposed solution. This lets reviewers decide whether the solution is appropriate instead of assuming it is.

## 2. Existing Functionality Investigation

Before reviewing the feature, check what the project already provides.

Check the existing code, public APIs, internal systems, platform implementations, and documentation.

Determine:

- Whether the functionality already exists.
- Whether an existing API can do the same thing.
- Whether another feature already solves part of the problem.
- Whether an existing system could be extended instead.
- Whether similar functionality exists under a different name.
- Whether there is an unfinished implementation or an earlier attempt.
- Whether the feature would duplicate something the project already does.

Include evidence for each relevant finding, such as file paths, class names, methods, or existing behavior.

Be clear about what already exists, what only exists in part, and what does not exist.

If the project already solves the problem, explain why another implementation is still needed.

## 3. Feature Value and Justification

Explain what the feature would add to the project and whether that is worth doing.

Evaluate:

- What can the project do after this feature that it cannot do now?
- Which current limitations would it remove?
- Which use cases would it cover?
- Would it improve the public API or make it more complicated?
- Does the feature belong in another existing system?
- Is the benefit worth the cost of building and maintaining it?
- Could an existing feature be extended to get the same result?
- What would happen if the feature were not implemented?

Consider other ways to solve the problem, including making no changes.

The fact that a feature is easy to implement is not enough reason to add it.

## 4. Weakness and Failure Analysis

Look for reasons not to approve the feature, and identify its weaknesses before deciding whether it is acceptable.

Consider:

- Missing functionality and incomplete behavior.
- Unhandled edge cases.
- Invalid or unexpected input.
- Resource ownership and lifetime management.
- Memory use and potential leaks.
- Thread safety and synchronization.
- Performance.
- Error handling and failure recovery.
- Unnecessary dependencies.
- API complexity and usability.
- Potential conflicts with existing systems.
- Future maintenance costs.
- Potential misuse by application developers.
- Security implications, where applicable.

For each weakness, include:

- **Description:** What is wrong, or what could go wrong?
- **Impact:** What could happen if the problem is not addressed?
- **Likelihood:** How likely is it to happen?
- **Severity:** How serious would the consequences be?
- **Resolution:** What needs to change to address it?

Do not dismiss a weakness just because it is unlikely.

If the available information is not enough to answer something, record it as an unresolved question. Do not assume it will work.

## 5. Platform Compatibility Review

State whether the feature is cross-platform or platform-specific.

For a cross-platform feature, review each of the three supported platforms separately:

- Windows
- Linux
- Android

A shared API does not mean the feature works on every platform.

For each platform, check:

- Is the required native functionality available?
- Which native APIs or system mechanisms would the feature need?
- Can the implementation use existing platform abstractions?
- Does it need additional platform-specific code?
- Do platform limitations change how it works?
- Do its lifecycle or threading requirements differ?
- Can the feature be tested on that platform?
- Would it add platform-specific dependencies?

Give each platform one of these compatibility statuses:

- **Full:** The feature can provide its intended functionality.
- **Adapted:** The feature works, but some implementation or behavior differs.
- **Unsupported:** The platform cannot provide the required functionality.
- **Deferred:** Support is possible, but the implementation is not currently planned.

For an adapted or unsupported platform, explain exactly what differs and what that means for users.

Do not claim full compatibility just because the feature can use a shared interface.

If the feature is only for one platform, explain why.

## 6. Architecture and Integration Review

Explain where the feature fits in the existing architecture and what it would affect.

Check:

- Which existing components it would affect.
- Where the implementation belongs.
- Whether it needs new abstractions or can reuse existing ones.
- Whether its dependencies fit the project's architecture.
- Whether platform-specific details stay in the right platform layer.
- Whether it adds interfaces or classes that are not needed.
- Whether existing APIs need to change.
- Whether it follows the project's ownership, threading, and lifecycle rules.

Identify architectural problems before implementation begins.

If the solution needs changes to unrelated systems, explain why.

## 7. Public API and Developer Experience Review

If the feature adds or changes a public API, review the API separately from its implementation.

Consider:

- Does its name fit the existing APIs?
- Is it easy to use?
- What configuration does it need?
- What does it do by default?
- How is it expected to be used?
- How does it report errors?
- Does it expose implementation details that users do not need?
- Can developers use it without workarounds?
- Could it cause ambiguous or conflicting behavior?

Include an example when it would help explain how to use the API.

An API is not ready just because its implementation is technically correct.

## 8. Testing and Verification Plan

Decide how the feature will be tested before approving implementation.

List the tests it needs:

- Unit tests.
- Integration tests.
- Platform-specific tests.
- Edge-case tests.
- Invalid input tests.
- Failure and recovery tests.
- Regression tests.

For a cross-platform feature, say which tests need to run on Windows, Linux, and Android.

Also explain:

- What existing behavior could break.
- Which existing tests may need changes.
- Whether the current testing setup can test the feature.
- Whether more testing infrastructure is needed.
- How the expected behavior will be checked.

The feature is not ready for approval if there is no clear way to verify it.

## 9. Independent Reviews

Have the proposal reviewed independently by multiple reviewers.

Each reviewer must review the original proposal and its evidence, not just rely on another reviewer's conclusions.

Each reviewer should check:

- Is the problem understood correctly?
- Would the proposed solution solve it?
- Was existing functionality checked thoroughly enough?
- Is there a better solution?
- Did the proposal miss any architectural problems?
- Are the platform compatibility claims correct?
- Did the proposal identify all the weaknesses?
- Are there other risks?
- Is the proposed API appropriate?
- Is the feature worth implementing?

Reviews must include specific findings, not just general opinions.

Give each finding one of these categories:

- **Blocking:** The feature cannot be approved until this is resolved.
- **Major:** A significant weakness that needs a correction or an explicit decision.
- **Minor:** A smaller concern that should be addressed or documented.
- **Informational:** An observation that does not prevent approval.

Include evidence for findings whenever possible.

If Hyper and Visor review the proposal, Visor must independently check any unresolved findings from Hyper. Visor must verify or challenge each finding with evidence, or resolve it, before the review can close.

## 10. Finding Resolution

Collect the findings from all independent reviews in one review record.

Respond to every finding with one of these decisions:

- **Fixed:** The proposal has been changed to resolve it.
- **Accepted:** The concern is valid, but the consequences are understood and accepted by the decision-maker.
- **Rejected:** The finding is incorrect, with an explanation and supporting evidence.
- **Unresolved:** More investigation is required.

Blocking findings must be resolved before approval.

Do not ignore major findings. Explain how each one was addressed or accepted.

If a change affects the feature's behavior, architecture, API, or platform compatibility, review the affected sections again.

Repeat the review until the proposal is complete enough and its remaining risks are understood.

## 11. Final Approval

After the reviews, give the proposal one of these decisions:

- **Approved:** The feature can proceed to implementation.
- **Revision Required:** The proposal needs more changes before another review.
- **Rejected:** The feature should not be implemented in its current form.
- **Deferred:** The feature may be reconsidered later.

Approve the proposal only when it has:

- A clear problem and justification.
- A completed investigation of existing functionality.
- A sufficiently detailed design.
- An acceptable weakness analysis.
- A completed platform compatibility review, when applicable.
- An acceptable architecture and API design.
- A defined testing strategy.
- No unresolved blocking findings.
- A review record containing all relevant decisions.

The project developer makes the final decision. Reviewers provide findings and recommendations, but cannot approve implementation on their own.

## 12. Implementation Review

Approving the proposal does not automatically approve the implementation.

After implementation, check that:

- It follows the approved design.
- It does not add unapproved functionality.
- It does not unintentionally change existing behavior.
- It supports each required platform as agreed in the compatibility review.
- The planned tests have been added and run.
- Relevant regression tests pass.
- The documentation matches the implementation.
- The final code review has no unresolved blocking findings.

If implementation needs significant changes from the approved proposal, review the affected design sections again.

## 13. Final Acceptance

The feature is complete only after its implementation has been reviewed and accepted.

The final review record must include:

- The original feature proposal.
- Existing functionality investigation.
- Feature justification.
- Identified weaknesses and their resolutions.
- Platform compatibility decisions.
- Architecture and API decisions.
- Independent reviewer findings.
- Final approval decision.
- Implementation verification results.

Do not consider the feature complete until it has been accepted.
