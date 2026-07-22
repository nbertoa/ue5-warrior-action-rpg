# Project setup

## Requirements

- Unreal Engine 5.6.
- Visual Studio 2022 with the C++ game-development workload, or JetBrains Rider.
- The plugins enabled in `WarriorRPG.uproject`: Common UI, Gameplay Abilities,
  Motion Warping, and BertaDevKit. Enhanced Input and Niagara are module
  dependencies used by the game code.

## Open and build

1. Open `WarriorRPG.uproject` with Unreal Engine 5.6, or generate IDE project files.
2. Build the `WarriorRPGEditor` target for `Win64 Development`.
3. Open the editor and load the map configured in **Project Settings > Maps & Modes**.

Example build command on Windows:

```powershell
& 'E:\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat' `
  WarriorRPGEditor Win64 Development `
  -Project='E:\Unreal Projects\ue5-warrior-action-rpg\WarriorRPG.uproject' `
  -WaitMutex -NoHotReload
```

## Required editor configuration

The C++ classes provide behavior; project assets supply concrete data. Verify the
following before testing a gameplay feature:

| Area | Required configuration |
| --- | --- |
| Hero pawn | Blueprint derived from `AWarriorHeroCharacter`, with `InputConfigDataAsset` assigned. |
| Enemy pawn | Blueprint derived from `AWarriorEnemyCharacter`, with `CharacterStartupData`, AI controller, widget class, collision settings, and hand-bone attachment names assigned as applicable. |
| Startup data | Hero/enemy startup data must grant the expected Gameplay Abilities and initialization Gameplay Effects. |
| Input | `DefaultInputComponentClass` must remain `WarriorRPGInputComponent`; configure the default mapping context and tagged input actions in `UDataAsset_InputConfig`. |
| UI | Register widget classes by gameplay tag in **Project Settings > Warrior UI Settings** before requesting a widget by tag. |
| Target lock | Assign object trace channels, `TargetLockWidgetClass`, and `TargetLockMappingContext` in the target-lock ability Blueprint. |
| Navigation | Add and build a NavMesh for any level that uses enemy summoning. |

## Fast validation checklist

- Start as the hero and confirm move/look input works.
- Trigger an ability granted by startup data.
- Verify health/rage UI receives attribute updates.
- Spawn an enemy on a NavMesh and confirm it receives its startup data after AI possession.
- Test target lock with a valid target and with no target.
- Test projectile behavior for both `OnHit` and `OnBeginOverlap` policies.

## Common failures

- Missing startup data: the character will have no configured abilities or attributes.
- Missing UI settings entry: `GetWidgetClassByTag` reports a setup error.
- No valid NavMesh point: an enemy spawn slot is skipped and logged.
- Running a spawn task on a client: the task ends without creating local enemies; the server must own the spawn.
