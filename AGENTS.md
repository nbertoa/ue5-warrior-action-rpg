# AGENTS.md

# Canonical Source

This file is derived from my personal engineering knowledge base.

Canonical repository:

https://github.com/nbertoa/lifeos

Canonical AGENTS document:

https://github.com/nbertoa/lifeos/blob/main/AGENTS.md

When the canonical repository is accessible, consult it before making significant architectural decisions or when additional engineering guidance is needed.

If the canonical repository cannot be accessed, continue using the instructions in this file. Never assume missing guidance.

Changes to my long-term engineering standards should be made in the canonical LifeOS repository rather than independently in project repositories.

---

# Purpose

LifeOS is the canonical source of my long-term engineering standards, decision criteria, and AI collaboration guidance.

Your goal is not to maximize code output or documentation volume. Your goal is to improve engineering quality while preserving correctness, maintainability, and durable knowledge.

Do not turn repositories into encyclopedias. Prefer preserving judgment over collecting information.

# Language

Communicate with the user in Spanish by default.

However:

- Keep source code in English.
- Do not translate identifiers, Unreal Engine APIs, C++ keywords, class names, function names, variable names, modules, or technical terminology normally written in English.
- Preserve compiler errors, logs, and engine messages exactly as produced.
- Explain technical concepts in Spanish.
- Unless explicitly requested otherwise, all reviews, reasoning, summaries, architectural discussions, and progress updates must be written in Spanish.

# Instruction hierarchy

Follow this priority order:

1. Explicit user instructions for the current task.
2. Project requirements.
3. More specific AGENTS.md files.
4. This AGENTS.md.
5. Canonical LifeOS repository.
6. General engineering conventions.

If two instructions conflict, explain the conflict instead of silently choosing one.

# Collaboration workflow

Always follow this process:

Understand → Constrain → Design → Implement → Verify → Review → Integrate → Learn

Discussion is not authorization to modify files.

Authorization to modify files is not authorization to commit or push.

Before implementing:
- Understand the problem.
- Understand expected behavior.
- Identify ownership.
- Identify lifecycle.
- Identify invariants.
- Identify failure paths.
- Evaluate architectural impact.

Avoid speculative refactoring and unrelated cleanup.

Always review the complete diff before finishing.

# Engineering priorities

1. Correctness
2. Ownership and responsibility
3. Simplicity
4. Maintainability
5. Observability
6. Extensibility when justified
7. Performance when supported by evidence
8. Style

# Design principles

- Prefer cohesive responsibilities.
- Keep ownership explicit.
- Keep lifecycle explicit.
- Minimize coupling.
- Prefer composition over inheritance.
- Avoid speculative abstractions.
- Apply SOLID and Clean Code pragmatically.

# Defensive programming

Always distinguish:
- Preconditions
- Postconditions
- Invariants
- Expected runtime failures
- Programming defects
- Recoverable failures
- Non-recoverable failures

Prefer preventing invalid states through architecture.

Do not compensate for poor ownership with defensive checks.

Use early returns to clarify valid execution paths.

Choose deliberately between:
- validation
- check
- ensure
- error propagation
- fallback

Never silently continue from an unsafe state.

# Unreal Engine guidance

Always consider:
- UObject ownership
- Garbage Collection
- Actor lifecycle
- Component lifecycle
- Reflection
- UPROPERTY/UFUNCTION
- Blueprint exposure
- Module boundaries
- Replication
- Authority
- Delegates
- Timers
- Async work
- Game thread
- Asset references

Prefer C++ for architecture and systems.

Prefer Blueprint for composition and iteration.

Never invent Unreal APIs.

# Verification

Match verification effort to risk.

Never claim tests that were not executed.

Always report:
- what was verified;
- what could not be verified;
- remaining assumptions;
- remaining risks.

# Code review

Prioritize:
1. Correctness
2. Undefined behavior
3. Ownership
4. Invariants
5. Architecture
6. API design
7. Performance
8. Maintainability
9. Style

Explain consequences.

Distinguish defects from preferences.

# Git policy

Unless explicitly authorized:
- do not commit;
- do not push;
- do not create branches;
- do not open pull requests;
- do not discard user work;
- do not modify unrelated files.

Always summarize the final diff.

# Final standard

Understand before implementing.

Prefer the simplest design that is correct, maintainable and verifiable.

Preserve durable engineering knowledge.
