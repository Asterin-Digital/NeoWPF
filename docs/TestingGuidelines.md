# Feature Testing Guidelines

## 1. Feature Ownership

Every feature must have its own dedicated test file.

Example:

`tests/Automated/LocalizationTests.cpp`

All automated tests for that feature belong in that file. Tests must not be scattered across unrelated feature test files merely because they use the same class or utility.

A test file should cover the feature's complete public behavior, including its success paths, alternative outcomes, invalid inputs, and failure handling.

## 2. Behavioral Coverage

Every distinct, externally observable behavior must have at least one dedicated test case.

A test case must verify a specific behavior or contract. Tests must not simply call a function and assume success.

For each behavior, tests must cover every distinguishable outcome, including:

- Normal operation and successful results.
- Alternative valid inputs and execution paths.
- Empty, missing, invalid, and unsupported inputs.
- Boundary conditions and limits.
- Conflicting inputs and precedence rules.
- State changes, repeated calls, and recovery after failure.
- Expected errors, failure results, and side effects.
- Preservation of state when an operation fails.

Not every category applies to every feature. The test suite must cover all applicable cases.

## 3. Test Granularity

Each test case must focus on one behavioral rule or outcome.

Related input variations may share a parameterized test when they verify the same rule. Distinct behaviors must have separate test cases.

Tests must not combine unrelated behaviors into a single large test that makes failures difficult to identify.

Test names must describe the condition being tested and the expected result.

Example:

`ResolveLocale_UsesExactMatchWhenAvailable`

`ResolveLocale_UsesLanguageFallbackWhenExactMatchIsMissing`

`ResolveLocale_UsesConfiguredFallbackWhenLanguageMatchIsMissing`

`ResolveLocale_ReturnsFailureWhenNoFallbackExists`

## 4. Completeness

Before a feature is considered tested, its behavior must be inventoried against its public contract.

Every public operation, supported option, meaningful state transition, and documented failure condition must be accounted for.

Finite sets of outcomes should be tested exhaustively where practical. For unrestricted input spaces, tests must cover meaningful input categories, boundaries, and relevant invariants rather than attempting to enumerate every possible value.

Code coverage may help identify missing tests, but coverage percentages alone do not establish behavioral completeness.

## 5. Test Independence

Every test must produce a reliable result independently of execution order.

Tests must not depend on another test having initialized state, registered resources, or executed beforehand.

Tests must use deterministic inputs, isolate mutable state, and clean up resources they create. Time-based waiting, external services, and platform-dependent assumptions must not be introduced unless they are necessary to test the feature's contract.

## 6. Assertions

Every test must assert its expected outcome.

Assertions must verify the relevant return values, resulting state, errors, side effects, and preservation of existing state as applicable.

Tests should verify observable behavior rather than internal implementation details.

A feature's test suite is complete only when every identified behavioral requirement has corresponding verification and every discovered failure case has been addressed.

## 7. Older Tests

Older tests within `Automated` that were made before the testing guidelines we're designed are not subject to these rules and should not be modified unless there's a good reason.

If a test file contains both older tests that do not comply with these guidelines and newer tests that do, the entire file must be refactored to comply with the current guidelines.