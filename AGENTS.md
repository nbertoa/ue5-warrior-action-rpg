# Unreal Engine — AI Agent Conventions

## Purpose

This file defines how I want an AI coding agent (Codex, or any AGENTS.md-compatible agent) to work on my Unreal Engine
C++ projects. It combines general engineering judgment with Unreal-specific and Epic C++ coding standard conventions.

Source of truth: https://github.com/nbertoa/unreal-agents-conventions

When copying this file into a project, paste it as-is into that project's root `AGENTS.md`. Project-specific rules
(build commands, target platform, module names, third-party plugins) belong in that project's own `AGENTS.md`
additions, not here.

---

## 0. Working Rules

These are operational rules for how the agent works with me day to day — read them before touching any code.

### Language

Communicate in Spanish by default: explanations, reasoning, summaries, architectural discussion, and progress
updates are all in Spanish unless I explicitly ask otherwise.

Source code stays in English regardless of conversation language — identifiers, Unreal Engine APIs, C++ keywords,
class/function/variable names, modules, and technical terminology. Preserve compiler errors, logs, and engine
messages exactly as produced, never translated.

### Instruction hierarchy

When instructions could conflict, apply this order:

1. My explicit instructions for the current task.
2. Concrete project requirements and constraints.
3. A more specific local `AGENTS.md` (if the project has additions beyond this file).
4. This file.
5. General engineering conventions.

If two instructions genuinely conflict, surface the conflict instead of silently picking one.

### Two separate permission thresholds

- Discussing or analyzing something is **not** authorization to modify files.
- Authorization to modify files is **not** authorization to `git commit` or `git push`.

Treat these as independent gates.

### Verification without building

Do not compile, build, cook, package, or run any equivalent build-pipeline action for the project — I run those
myself and can share the results with you for diagnosis or follow-up. Static inspection and source-level analysis
remain fine and expected. Always state explicitly which verification was actually performed and which
compilation/build checks are still pending because of this constraint. Never claim a test or build was run when it
wasn't.

### Git policy

Unless I explicitly authorize it for that specific action: do not commit, do not push, do not create branches, do
not open pull requests, do not discard my uncommitted work, do not modify files unrelated to the current task.
Always summarize the final diff before finishing.

---

## 1. Engineering Priorities

Default order, used to break ties when two goals conflict:

1. Correctness
2. Clear responsibility and ownership
3. Simplicity
4. Maintainability
5. Observability and diagnosability
6. Appropriate extensibility
7. Performance when evidence makes it relevant
8. Style and polish

Convenience and elegance never outrank correctness. Simplicity means minimizing concepts, dependencies, and
hard-to-reason-about states — not minimizing lines of code.

### Core principles

- **Correctness first.** A readable or elegant implementation is not successful when its behavior is wrong. Define
  expected behavior and meaningful failure cases before polishing the design.
- **Reduce uncertainty before adding complexity.** Misunderstood requirements create more waste than a lack of
  abstraction. Ask questions, inspect the existing system, separate facts from assumptions. Never invent Unreal APIs
  or assume engine behavior — verify against current documentation or engine source when uncertain.
- **Prefer the simplest sound design.** Avoid speculative frameworks, undefined manager objects, and configuration
  added only to avoid a decision. Add complexity only when a real constraint requires it.
- **Optimize for readability and local reasoning.** Use clear names, cohesive functions, straightforward control
  flow. Comments explain *why*, not *what*. Avoid clever compression and hidden side effects.
- **Make responsibility, ownership, and lifecycle explicit.** In Unreal, this means explicitly accounting for UObject
  garbage collection, Actor/Component lifecycle, delegates, timers, async work, and editor versus runtime context.
- **Keep APIs narrow and contracts visible.** Every public operation creates coupling and a long-term expectation.
  Broader APIs are justified by real consumers, not hypothetical flexibility.
- **Separate concerns without chasing abstraction.** Prefer composition for independently varying capabilities and
  inheritance for genuine specialization or a framework contract. Accept limited duplication when a shared
  abstraction would blur responsibilities or predict variation that hasn't appeared yet.
- **Treat failure deliberately.** See §2 below.
- **Measure before optimizing.** Most projects are prototypes or MVPs — correctness, readability, and maintainability
  usually matter more than maximum performance. Optimize after validating the solution, unless there's an obvious
  regression (e.g. a frame-rate drop) worth investigating immediately.
- **Work in small, coherent increments.** Keep related work together, unrelated work separate. Preserve a valid
  intermediate/buildable state where practical.
- **Verify according to risk.** Compilation proves only part of correctness. "It compiled" is never sufficient
  evidence for a behavioral change.
- **Use AI without outsourcing judgment.** Ask for alternatives and explanations, verify technical claims, review
  generated code according to risk. Don't repeatedly apply generated fixes until a symptom disappears without
  understanding why.

---

## 2. Defensive Programming

Defensive programming is not "add checks everywhere." It's the discipline of making assumptions explicit, detecting
broken contracts close to their source, preventing invalid state from spreading, and choosing failure behavior
deliberately.

> Validate at trust boundaries, assert internal contracts, and design invalid states out where practical.

### Expected vs. unexpected failure

- **Expected failure** is a normal outcome callers can reasonably encounter: invalid user input, a failed network
  request, an absent optional asset, a lookup with no result, a gameplay rule rejecting an action. Represent it with
  an explicit result, optional value, error, or documented no-op.
- **Unexpected failure** is evidence an internal contract was broken: a required dependency wasn't initialized, an
  invariant no longer holds, an object violates its lifecycle. Surface it loudly and close to its source — continuing
  may spread corruption or conceal the actual defect.

Programming errors should never be hidden behind ordinary return values.

### Preconditions, postconditions, invariants

- **Precondition**: what a caller must provide before an operation runs.
- **Postcondition**: what the operation guarantees when it completes.
- **Invariant**: what must remain true for a valid object/subsystem during its lifetime.

The owner of a public/trust-boundary API validates or sanitizes untrusted input. The caller satisfies documented
preconditions. Internal, tightly controlled code may assert those preconditions instead of repeatedly validating
them.

### Prefer preventing invalid states over detecting them

Factories/constructors can establish validity before an object exists. Enums can replace loosely related booleans;
dedicated value types can restrict invalid values; explicit optionals can distinguish absence from a default. In
Unreal, staged initialization (constructor → `PostInitializeComponents` → `BeginPlay`) can make a fully valid object
unavailable at construction time — treat that transitional state as explicit and narrow, guarded at the boundary
where it becomes usable, not as a permanent excuse for null checks everywhere.

### Early returns and guard clauses

Useful for failed preconditions, unavailable optional dependencies, unsupported states, authorization failures. They
reduce nesting and keep the successful path visible:

```cpp
if (!IsValid(Target))
{
    return;
}
```

This is appropriate only when absence is expected and doing nothing is a real outcome. If `Target` is required at
this point, the same early return would suppress an invariant violation — log, assert, or return a meaningful
failure instead. **An early return is not automatically defensive when it hides a bug.**

### Unreal-specific mechanisms — choose by failure category, not interchangeably

- **`check` / `checkf`** — internal assumptions/invariants that must hold. Use when continuing would be invalid. Never
  for external, player, network, or content input.
- **`ensure` / `ensureMsgf`** — report an unexpected condition while allowing the operation to stop or degrade, only
  when valid state can be preserved. Not permission to continue blindly.
- **`ensureAlways`** — rare; only when repeated reporting is genuinely valuable and won't create noise.
- **`verify`** — for an expression that must execute in all relevant builds while its result is still validated in
  assertion-enabled configurations.
- **`IsValid`** — for object references that may legitimately be stale or pending destruction. Not a universal
  replacement for clear ownership/lifecycle contracts. Don't apply it mechanically to every pointer.
- **Logging + explicit return values** — for expected runtime outcomes; should tell the caller/operator what happened
  and what follows.

Account for Development, Debug, and Shipping build behavior — checks compiled out in Shipping cannot be a production
safeguard for content that must be validated at runtime.

### Common failure modes to avoid

- Returning silently on every invalid pointer (hides missing initialization/ownership defects).
- Validating the same invariant at every call layer instead of fixing the responsible contract.
- Using `ensure` for routine control flow, or `check` on external input.
- Logging an error and then continuing with invalid state.
- Swallowing errors with a default value that silently changes caller behavior.
- Creating partially initialized objects and treating their state as normal.
- Adding checks that no caller can act on.

---

## 3. Architecture & API Design

### Responsibilities

Keep classes small with a single, well-defined responsibility. When a class accumulates unrelated behavior, split it.
Avoid manager classes with undefined boundaries and utility APIs that accumulate arbitrary functionality.

### Public surface area

Every public symbol creates coupling, caller expectations, and maintenance cost. Prefer the smallest surface that
supports current requirements; private implementation by default; deliberate, justified exposure. Challenge: public
mutable state, getters/setters for every field, Blueprint exposure "just in case," convenience methods with no clear
responsibility.

### Ownership and lifetime

Make ownership explicit — who creates, who destroys, who may retain, whether a reference can disappear, whether
cleanup is symmetrical. In Unreal, account for:

- UObject garbage collection and `UPROPERTY` tracking
- Weak object references
- Actor and Component lifecycle
- Delegates, latent actions, async callbacks
- Editor-world versus runtime-world lifetime

Repeated null checks in trusted code are often a signal of unclear ownership or lifecycle design, not evidence of
good defensive programming.

### Types and invalid states

> Invalid states should be unrepresentable where the design cost is reasonable.

Enums are clearer than magic integers. Multiple independent booleans often permit contradictory combinations that a
single named state would not (e.g. `SetState(bool Enabled, bool Paused, bool Destroyed)` — prefer a state
enum/machine).

### Abstraction and extensibility

Abstract around **real, demonstrated** variation — not because a future implementation might appear, or because
"every class should have an interface." Duplication is often cheaper than the wrong abstraction. Inheritance is not
a default reuse tool; prefer composition unless there's genuine specialization or a framework contract (e.g. `UActorComponent`) that requires it.

### Blueprint-facing APIs

Blueprint exposure is API design, not annotation. Design for clear node names, safe defaults, minimal pin count,
meaningful categories, explicit authority expectations, world-context clarity, predictable failure, limited surface
area. Do not expose implementation details or make every property `EditAnywhere | BlueprintReadWrite` by default —
grant only the access actually required. See §7 for the exact specifier semantics.

---

## 4. Blueprints vs. C++

Neither should be used exclusively as a matter of principle. The right choice depends on project stage, team size,
system responsibility, and long-term maintenance cost.

### Use Blueprints for

- Fast solo prototyping and exploring an idea.
- Content configuration, asset assignment, class selection.
- Visual/gameplay-event wiring, presentation, project-specific variation.
- Rapid designer iteration.

### Use C++ for

- Collaborative work (text-based source merges/reviews/diffs far better than Blueprint assets).
- Plugins, core project systems, subsystems, reusable frameworks.
- Low-level functionality, external service/API integrations, systems needing third-party libraries.
- Infrastructure shared across multiple features.
- Anything requiring strong control over initialization, lifecycle, error handling, networking, or dependencies.

### The combined model

C++ defines the structural foundation: architecture, core rules, shared behavior, stable interfaces, protected
implementation details. Blueprints configure and specialize on top of that foundation: values, assets, project-specific
variation, gameplay event wiring. C++ exposes what designers genuinely need; it protects the invariants that keep the
system valid.

### Don't rewrite working Blueprint systems without a reason

If a Blueprint system works correctly, meets performance requirements, is understandable, maintainable, and doesn't
create collaboration problems — leave it. Migration should be driven by a concrete problem or benefit, not by "it
should be C++."

Note: AI-assisted development has reduced the historical speed advantage Blueprints had — AI makes it faster to
explore APIs, generate initial C++, and understand engine systems. This makes C++ more practical for rapid
development than before, especially for team-maintained code. It doesn't remove the need to understand and review
what's generated.

---

## 5. Gameplay Architecture

Applies when designing Character/Component/Ability systems. Design for the actual project's requirements — don't add
multiplayer complexity to a single-player prototype in advance; adapt proven patterns instead of copying them
mechanically.

- **Keep the Character focused on framework integration**, not as the owner of every mechanic. Persistent systems
  with their own state and operations (inventory, health, stamina) belong in components when that separation
  reflects their actual responsibility.
- **Gameplay Ability System**, when used, should coordinate an action — not become a container for unrelated rules,
  movement, presentation, targeting, and special cases. Costs, cooldowns, state changes, and presentation need clear
  homes instead of scattered custom logic.
- **Gameplay Tags** are a shared vocabulary for things that need to be queried, communicated, allowed, or blocked.
  Valuable when they reduce direct dependencies; require consistent naming; shouldn't become an ungoverned
  replacement for clear design.
- **Support safe configuration and variation** — a well-designed mechanic exposes appropriate configuration without
  requiring a C++ change for every variation. A new variant should reuse the common foundation and modify only what
  differs.

Before considering a mechanic well-integrated, check: responsibilities are explicit with no unnecessary duplicated
logic; the Character and individual abilities remain focused; state/presentation/configuration aren't accidentally
coupled; tags are used consistently; important invariants remain protected; a meaningful variant can be added without
rewriting the original mechanic.

---

## 6. Epic C++ Coding Standard

These rules follow Epic's official C++ Coding Standard, plus explicit additions where noted. They are mandatory, not
guidelines.

### Class organization

`public` → `protected` → `private`, in that order. Never invert it.

### Naming conventions

- **PascalCase** for all type names, variables, and methods. No underscores between words.
- Type prefixes, enforced by UnrealHeaderTool:
  - `T` — template classes
  - `U` — UObject subclasses
  - `A` — AActor subclasses
  - `S` — SWidget subclasses
  - `I` — abstract interfaces
  - `E` — enums
  - `F` — structs and most other classes
  - `b` — boolean variables (`bIsAlive`, `bHasFired`)
  - `UE_` — macros
- Type names are **nouns**; method names are **verbs** or verb phrases.
- Boolean functions ask a true/false question: `IsVisible()`, `ShouldClearBuffer()`.
- Output parameters get an `Out` prefix: `OutResult`, `bOutSuccess`.
- Names are clear, unambiguous, descriptive — avoid abbreviation.
- One variable declaration per line, so each can carry its own comment.
- Never shadow a variable from an outer scope, even where the compiler allows it.

### Const correctness

`const` is documentation as much as a compiler directive.

- Pass arguments by `const` pointer or `const` reference when they're not modified.
- Flag methods `const` when they don't modify the object.
- Use `const` iteration over containers that aren't mutated in the loop.
- Prefer `const` for by-value parameters/locals that aren't reassigned.
- `const` goes on the right of `*` for pointer-is-const: `T* const Ptr` (const pointer, non-const `T`).
- **Never `const` on a return-by-value type** — it inhibits move semantics:
  ```cpp
  const TArray<FString> GetArray();          // Bad — inhibits move
  const TArray<FString>& GetArray() const;   // Fine — const ref
  const TArray<FString>* GetArray() const;   // Fine — const pointer
  ```

### Comments

- Write self-documenting code first — a well-named variable needs no comment.
- Comments document *intent* (the why); code documents *implementation* (the what).
- Don't over-comment bad code — rewrite it instead. Don't contradict the code.
- **Doxygen is required for all publicly declared functions**, written at the declaration site in the `.h`, never in
  the `.cpp`. Includes public getters, virtuals, and any new function added during review. Private functions get
  Doxygen when purpose isn't self-evident from the name alone.
- Inline comments in `.cpp` bodies explain the *why* behind non-trivial blocks, not the *what* the code already
  expresses.
- `@warning`, `@note`, `@see`, `@deprecated` each go on their own line after the main description.

### Float and double literals (addition to Epic standard)

Always write the leading zero before the decimal point: `0.0f` not `.0f`/`.f`, `1.0f` not `1.f`, `180.0f` not
`180.f`. Same for doubles: `0.0` not `.0`.

### Modern C++ — allowed and forbidden

C++17 is the baseline. UE5.4+ exposes select C++20 features where Epic has validated them — don't assume C++20 is
available across all target platforms (console SDKs can lag).

| Feature | Rule |
|---|---|
| `nullptr` | Always. Never `NULL`. |
| `override` + `virtual` | **Both required** on every override in derived classes. |
| `final` | Use when the class isn't designed to be derived from. |
| `static_assert` | Valid anywhere a compile-time assertion is needed. |

### UObject / UPROPERTY / garbage collection

Always wrap `UObject`-derived pointers in `UPROPERTY()` so the garbage collector tracks them — including
`TObjectPtr` members and raw pointer members:

```cpp
// Bad — GC will collect MeshComponent
UStaticMeshComponent* MeshComponent;

// Correct — GC tracks it
UPROPERTY()
TObjectPtr<UStaticMeshComponent> MeshComponent;
```

### UCLASS specifiers — deliberate, not default

- `Blueprintable` — allows Blueprint subclasses. Omit for C++-only classes.
- `NotBlueprintable` — explicitly blocks Blueprint subclassing.
- `BlueprintType` — allows the type as a Blueprint variable. Required even without `Blueprintable` if Blueprints need
  to hold a reference.
- `Abstract` — prevents direct instantiation; use for base classes never meant to be placed in a level.
- `ClassGroup=(GroupName)` — controls grouping in the Add Component menu. Always set for components.
- `HideCategories=(Category)` — removes inherited Detail panel categories irrelevant to this class.

Don't accumulate specifiers over time without reviewing whether each is still needed.

### Property/function exposure specifiers

`EditAnywhere` and `BlueprintReadWrite` are orthogonal — choose independently:

- `EditAnywhere` — editable in the Details panel (archetype and instance). Designer-tunable values.
- `EditDefaultsOnly` — editable only on the class default object. Preferred when instance-level editing isn't needed.
- `BlueprintReadWrite` — readable and writable from Blueprint graphs. Reserve for values Blueprints genuinely need to
  set.
- `BlueprintReadOnly` — readable from Blueprints, not writable. For observable state that C++ owns.
- Always add `meta=(AllowPrivateAccess="true")` when exposing a `private` member to preserve C++ encapsulation.
- Avoid defaulting to `EditAnywhere | BlueprintReadWrite` for every property — grant only the access actually
  required.

`UFUNCTION`: `BlueprintCallable` for BP access, `BlueprintImplementableEvent` for BP-only overrides,
`BlueprintNativeEvent` for C++/BP hybrid.

### Subsystems

Use `UGameInstanceSubsystem`, `UWorldSubsystem`, `ULocalPlayerSubsystem` for singleton-like services instead of ad-hoc
globals or manager objects.

### `GetWorld()` safety

Never call `GetWorld()` in a constructor — no world context yet, returns `nullptr`. Avoid it in UObjects that aren't
Actors/ActorComponents; use `GEngine->GetWorldFromContextObject()` with a proper world context object instead. Always
guard the result: `if (UWorld* World = GetWorld())`.

### Delegates

Prefer named member callbacks over lambdas: `CreateUObject(this, &ThisClass::MyCallback)` with a dedicated private
method.

- **Multicast delegates** (`AddUObject`) — the engine checks object validity before invocation; no manual
  `TWeakObjectPtr` needed.
- **Single-cast delegates** (`BindUObject`) and **timer callbacks** — the engine does *not* always guard against a
  destroyed object. Clear those delegates explicitly in `EndPlay`/`BeginDestroy`, or use `BindWeakLambda` /
  `CreateWeakLambda` when the binding must outlive the owner.

Only use plain lambdas when the body is trivial (1–2 lines) and a named method adds no clarity.

### Logging

Every system gets a custom log category (`DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY`). Never use
`LogTemp`. Include the actor name and relevant values in every log line:

```cpp
UE_LOG(LogMySystem, Warning, TEXT("[%s] Health dropped to %d"), *GetName(), CurrentHealth);
```

**Always dereference `FString` with `*` when passing to `%s`** — passing it undereferenced is a silent bug (garbage
or crash), not a compiler error:

```cpp
// Bad — undefined behavior, no compiler error
UE_LOG(LogMySystem, Warning, TEXT("Actor: %s"), MyFString);

// Correct
UE_LOG(LogMySystem, Warning, TEXT("Actor: %s"), *MyFString);
```

Applies to every `%s` in `UE_LOG`, `FString::Printf`, and `ensureMsgf`. `FString::Printf`'s *result*, when passed to
another `%s`, must also be dereferenced.

### `ThisClass`

`GENERATED_BODY()` injects `ThisClass` as a typedef for the current class. Prefer `&ThisClass::MyMethod` over
`&AMyClass::MyMethod` in method pointers — survives class renames, matches Epic's internal style.

---

## 7. UE5-Specific Lifecycle & Pitfalls

### AActor initialization order

1. **Constructor** — `CreateDefaultSubobject` calls, default values. No world context. Never call `GetWorld()`,
   `GetOwner()`, or access other actors here.
2. **`PostInitializeComponents()`** — all components created and initialized. Component methods are safe. Still no
   gameplay world.
3. **`BeginPlay()`** — actor fully spawned and registered. Start gameplay logic, bind delegates, set timers, call
   `GetWorld()` freely.
4. **`Tick(float DeltaTime)`** — per-frame. Only enable with `PrimaryActorTick.bCanEverTick = true` in the
   constructor; disable if unused.
5. **`EndPlay(EEndPlayReason::Type)`** — actor being removed from the world. Clear timers, unbind delegates, release
   handles. Called for every removal reason (destroyed, level transition, PIE end).
6. **`Destroyed()`** — only for explicit destruction (not level unload). Prefer `EndPlay` for cleanup — it covers all
   exit paths.

### UActorComponent initialization order

1. **Constructor** — same rules as AActor. `GetOwner()` returns `nullptr` here.
2. **`OnRegister()`** — component registered with the world. `GetOwner()` is safe. Use for setup that depends on the
   owner but must happen before `BeginPlay`.
3. **`InitializeComponent()`** — only if `bWantsInitializeComponent = true`. Rarely needed; prefer `BeginPlay`.
4. **`BeginPlay()`** — mirrors AActor's `BeginPlay`.
5. **`EndPlay()`** — mirrors AActor's `EndPlay`. All cleanup goes here.

### `CreateDefaultSubobject` vs. `NewObject`

- `CreateDefaultSubobject<T>(TEXT("Name"))` — **only in the constructor**. Registers with the CDO, appears in the
  Details panel, participates in serialization.
- `NewObject<T>(this, T::StaticClass())` — **at runtime** (BeginPlay or later). Must be manually registered
  (`RegisterComponent()` for scene components). Doesn't appear in the editor CDO.
- Never swap these: `CreateDefaultSubobject` outside the constructor crashes; `NewObject` in the constructor produces
  untracked objects.

### Pitfalls checklist

| Pitfall | Silent consequence |
|---|---|
| `Cast<>` when exact type needed | Succeeds for subclasses too — not exact. Use `ExactCast<>` |
| `GetWorld()` in constructor | Returns `nullptr` — no world context yet |
| `TWeakObjectPtr` used without `IsValid()` check | Stale pointer after GC — crash |
| Delegate not cleared in `EndPlay` | Callback fires on a destroyed object |
| `LogTemp` anywhere | Unfiltered noise, impossible to disable per system |
| `EditAnywhere \| BlueprintReadWrite` on every property | Designers can break C++ invariants |
| Range-for without `&` on `TArray` | Iterates a silent copy — mutations are lost |
| `SpawnActorDeferred` without `FinishSpawning` | Actor stuck in limbo — `BeginPlay` never fires |
| Raw `AActor*` in a `UPROPERTY`-less struct | GC collects it — dangling pointer |
| `NewObject` with null or wrong outer | GC cannot track it — premature collection |
| `TArray` UPROPERTY without `EditFixedSize` on invariant arrays | Designer adds/removes elements, breaks assumptions |
| Delegate callback method missing `UFUNCTION()` | Reflection fails — crash or silent no-op in packaged build |
| `Super::EndPlay()` called too early | Base class releases resources the derived class still holds |
| `CreateDefaultSubobject` component missing `UPROPERTY()` | GC collects the component — dangling pointer |

Flag these when code approaches them — don't warn preemptively about hypothetical issues.

---

## 8. Code Review & Debugging Checklists

### Code review order

Review in this order — style comes last:

1. Correctness (including edge cases and failure paths, not just the happy path)
2. Crashes, corruption, undefined behavior
3. Ownership and lifecycle
4. Invariants and failure handling
5. Architecture and responsibilities
6. API design and coupling
7. Concurrency, replication, performance
8. Maintainability and readability
9. Style

Distinguish defects from preferences. Each finding should explain its concrete consequence — not "this feels wrong,"
but "this callback captures the owning object strongly; if the async operation outlives the owner, the lifetime
becomes unclear." Don't approve a change merely because it looks clean; don't block on speculative perfection either.

For Unreal changes specifically, review: engine lifecycle usage; Actor vs. Component responsibility; UObject
ownership/validity; Blueprint/C++ boundary; reflection exposure; replication and authority; module dependencies;
editor-only vs. runtime code; shipping-build behavior.

### Debugging discipline

Debugging is the disciplined reduction of uncertainty, not random modification until a symptom disappears.

- **Observe before modifying.** Preserve a known reproduction while investigating.
- **Define the failure precisely** — expected vs. actual behavior, environment, first occurrence, frequency,
  reproducibility. "It doesn't work" or "Unreal is broken" are starting observations, not definitions.
- **Reproduce with the smallest failing case.** For intermittent failures, change one variable at a time.
- **Form falsifiable hypotheses** — what evidence should exist if correct, what shouldn't, what experiment
  distinguishes it from alternatives. Actively seek disconfirming evidence, not just confirming.
- **Find the earliest point actual behavior diverges from expected** — the final symptom is often far from the
  defect.
- **Account for time and lifetime explicitly** — many hard Unreal bugs are temporal: stale references, async
  callbacks, race conditions, delayed init, teardown order, replication/event order.
- **Identify the root cause, not just the trigger.** A recent change or specific map may be the trigger or amplifier,
  not the cause. Test removing the suspected condition and predict the outcome; look for cases where the condition
  exists without the failure.
- **Verify the fix** against the original reproduction, repeated execution for intermittent bugs, and relevant build
  contexts (PIE / Standalone / packaged). Recompilation alone never verifies a behavioral fix.

For Unreal specifically: account for Actor/Component lifecycle, construction vs. runtime, PIE vs. standalone vs.
packaged behavior, garbage collection, stale UObject references, delegates, latent actions, replication
authority/client-server state, and initialization order. Don't assume engine behavior from memory when current
source or docs can verify it.

---

## Final Standard

Understand the problem before implementing. Make contracts, ownership, lifecycle, and failure behavior explicit.
Choose the simplest design that is correct and appropriately resilient. Verify according to risk, not by default.
Review engineering decisions, not only style. Never invent Unreal APIs or engine behavior — verify against current
official documentation or engine source when uncertain.
