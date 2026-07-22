# Contributing

## Scope and conventions

- Prefer one focused change per commit.
- Keep public headers documented by purpose, parameters, return values, and
  non-obvious lifecycle or authority requirements.
- In `.cpp` files, comment decisions and edge cases rather than restating the
  next line of code.
- Use native gameplay tags from `WarriorRPGTags`; do not add raw tag strings.
- Use `ensure` for recoverable asset/configuration failures and `check` only for
  genuine programming invariants.
- Preserve server authority for actor spawning, Gameplay Effect application, and
  persistent game state.

## Before submitting

1. Format the touched files using the project IDE settings.
2. Build `WarriorRPGEditor Win64 Development`.
3. Run the affected feature in editor/PIE.
4. If the change affects networked gameplay, test a server and client PIE session.
5. Run `git diff --check` to detect whitespace errors.

## Commit messages

Use a concise conventional prefix when practical:

```text
fix(gameplay): prevent duplicate enemy spawn batches
feat(ui): add pause-menu widget stack
docs(gas): explain damage execution flow
```

## Documentation updates

Update the relevant document whenever a change alters setup requirements,
authority behavior, native tags, Data Asset shape, or a public C++ contract.
