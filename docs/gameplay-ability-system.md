# Gameplay Ability System

## Main types

| Type | Responsibility |
| --- | --- |
| `UWarriorAbilitySystemComponent` | Tag-based ability input, hero weapon ability grants, and activation by ability tag. |
| `UWarriorAttributeSet` | Health, rage, attack, defense, and transient damage attributes. |
| `UWarriorGameplayAbility` | Shared ability helpers and lifecycle behavior. |
| `UWarriorHeroGameplayAbility` / `UWarriorEnemyGameplayAbility` | Character-specific accessors and combat helpers. |
| `UGEExecCalc_DamageTaken` | Calculates final damage and writes it to `DamageTaken`. |

## Attribute flow

`UGEExecCalc_DamageTaken` captures source `AttackPower` and target `DefensePower`,
reads the set-by-caller damage values, and writes the final result to
`DamageTaken`. `UWarriorAttributeSet::PostGameplayEffectExecute` consumes that
transient value, reduces `CurrentHealth`, updates UI delegates, and adds the dead
status tag when health reaches zero.

`DefensePower` is clamped to a minimum divisor of `1.0` by the execution
calculation. Initialization Gameplay Effects should still set meaningful attack,
defense, health, and rage values for every character.

## Tags and input

Native tags live in `WarriorRPGTags.h/.cpp`. Keep tag names centralized; do not
introduce string literals for gameplay tags in new C++ code.

Input actions are assigned tags in `UDataAsset_InputConfig`. The ASC interprets
them as follows:

| Input model | Rule |
| --- | --- |
| Normal | Press tries to activate the matching ability. |
| Toggleable | Press activates an inactive ability or cancels an active one. |
| Held | Release cancels the matching active ability. |

Weapon abilities are granted by `GrantHeroWeaponAbilities` and removed through
`RemoveGrantedHeroWeaponAbilities`. Preserve their returned spec handles so the
same abilities can be removed when the weapon is unequipped.

## Creating a damage ability

1. Create a Gameplay Ability derived from the appropriate Warrior base class.
2. Grant it through hero/enemy startup data or weapon data.
3. Give it an owned ability tag and, if input-driven, a tagged input action.
4. Create a Gameplay Effect using `UGEExecCalc_DamageTaken` when custom damage
   calculation is required.
5. Set the base damage and optional attack-type magnitudes as set-by-caller values.
6. Apply the spec through `UWarriorFunctionLibrary` to preserve the project-wide
   ASC validation path.

## Ability tasks

`UAbilityTask_ExecuteTaskOnTick` provides a per-frame delegate while the owning
ability remains valid. `UAbilityTask_WaitSpawnEnemies` waits for a gameplay event,
asynchronously loads an enemy class, and spawns one authoritative batch at
NavMesh-reachable locations.
