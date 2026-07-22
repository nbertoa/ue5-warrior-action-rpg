# WarriorRPG — Unreal Engine 5.6 C++

> ⚠️ **Work in progress.** This project is actively being developed. Systems documented here reflect the current state of the codebase, not the final design.

A third-person action RPG built in Unreal Engine 5.6 using C++ throughout. The project implements a full combat pipeline powered by the Gameplay Ability System — melee combos, directional rolling, blocking, parrying, target locking, ranged enemies with projectiles, a boss character with summoning abilities, and a survival game mode with enemy waves. UI is handled entirely through Common UI with a custom input routing layer. AI uses behavior trees, environment query system, and crowd following for avoidance.

Built following the [Unreal Engine 5 C++: Advanced Action RPG](https://www.udemy.com/course/unreal-engine-5-advanced-action-rpg/) course by Vince Petrelli as a structural foundation, with my own C++ implementation, architectural decisions, and corrections throughout.

---

## Tech Stack

|  |  |
| --- | --- |
| **Engine** | Unreal Engine 5.6 |
| **Language** | C++ with Blueprint subclassing for asset configuration |
| **Plugins** | Gameplay Ability System, Enhanced Input, Common UI, Common Input, Motion Warping, Niagara |
| **Editor Plugins** | [Electronic Nodes](https://www.unrealengine.com/marketplace/en-US/product/electronic-nodes), [Blueprint Assist](https://www.unrealengine.com/marketplace/en-US/product/blueprint-assist) |
| **IDE** | JetBrains Rider |

---

## Documentation

The README describes the architecture. Use these practical guides when configuring
or extending the project in Unreal Editor:

- [Project setup](docs/setup.md) - required project settings, C++/Blueprint pairings, and validation.
- [Gameplay Ability System](docs/gameplay-ability-system.md) - attributes, tags, abilities, effects, and input routing.
- [Networking](docs/networking.md) - authority rules for combat, projectiles, and enemy spawning.
- [Contributing](docs/contributing.md) - code conventions and local verification commands.

---

## Architecture Overview

### Character Hierarchy

```
ACharacter + IAbilitySystemInterface + IPawnCombatInterface + IPawnUIInterface
  └── AWarriorBaseCharacter
        │   ASC, AttributeSet, MotionWarpingComponent, CharacterStartupData (TSoftObjectPtr)
        │
        ├── AWarriorHeroCharacter
        │     CameraBoom, FollowCamera, HeroCombatComponent, HeroUIComponent,
        │     InputConfigDataAsset
        │
        └── AWarriorEnemyCharacter
              EnemyCombatComponent, EnemyUIComponent, EnemyHealthWidgetComponent,
              LeftHandCollisionBox, RightHandCollisionBox, AutoPossessAI
```

### Controller Hierarchy

```
AWarriorHeroController   — IGenericTeamAgentInterface (TeamID=0)
                           Creates UWarriorPrimaryLayout before Super::OnPossess
                           so UI is available when OnGiven abilities activate

AWarriorAIController     — UCrowdFollowingComponent, UAIPerceptionComponent (sight)
                           IGenericTeamAgentInterface (TeamID=1)
                           OnEnemyPerceptionUpdated writes TargetActor to Blackboard
```

### Combat Component Hierarchy

```
UPawnExtensionComponentBase
  └── UPawnCombatComponent
        │   Weapon registry (TMap<FGameplayTag, AWarriorWeaponBase*>)
        │   ToggleWeaponCollision, OnHitTargetActor, OnWeaponPulledFromTargetActor
        │
        ├── UHeroCombatComponent
        │     GetHeroCurrentEquippedWeapon, GetHeroCurrentEquippedWeaponDamageAtLevel
        │     OnHitTargetActor → SendGameplayEvent MeleeHit + HitPause
        │
        └── UEnemyCombatComponent
              OnHitTargetActor → block evaluation (tag + geometry)
              Unblockable bypass via Enemy::Status::Unblockable
              ToggleBodyCollisionBoxCollision (left/right hand)
```

### UI Hierarchy

```
UWarriorUISubsystem (UGameInstanceSubsystem)
  └── UWarriorPrimaryLayout (UCommonUserWidget)
        ├── Background  (Queue)
        ├── Hud         (Queue)
        ├── Modal       (Stack)
        ├── Toasts      (Queue)
        ├── Toasts2     (Queue)
        └── Toasts3     (Queue)

UWarriorActivatableWidget (UCommonActivatableWidget)
  └── Base for all widgets — controls input mode per widget via EWarriorWidgetInputMode

UWarriorActionRouter (UCommonUIActionRouterBase)
  └── ApplyUIInputConfig body intentionally empty — prevents spurious Menu mode from Common UI

UWarriorUISettings (UDeveloperSettings, Config=Game)
  └── Tag → widget class map, accessed via UWarriorFunctionLibrary::GetWidgetClassByTag
```

### UI Component Hierarchy

```
UPawnExtensionComponentBase
  └── UPawnUIComponent
        │   OnCurrentHealthChanged (FOnPercentChangedDelegate)
        │
        ├── UHeroUIComponent
        │     OnCurrentRageChanged
        │     OnEquippedWeaponChanged (FOnEquippedWeaponChangedDelegate)
        │
        └── UEnemyUIComponent
              RegisterEnemyDrawnWidget / RemoveEnemyDrawnWidgetsIfAny
```

---

## Key Systems

### Gameplay Ability System

`UWarriorAbilitySystemComponent` extends the engine ASC with tag-based input routing supporting three models:

| Model | Behavior |
| --- | --- |
| Normal | Press activates |
| Toggleable | Press activates or cancels (matched via `Input::Toggleable` parent tag) |
| Held | Release cancels (matched via `Input::MustBeHeld` parent tag) |

`GrantHeroWeaponAbilities` and `RemoveGrantedHeroWeaponAbilities` manage the weapon ability lifecycle. `TryActivateAbilityByTag` selects randomly among matching specs for variety.

`UWarriorAttributeSet` manages `CurrentHealth`, `MaxHealth`, `CurrentRage`, `MaxRage`, `AttackPower`, `DefensePower`, and `DamageTaken`. `PostGameplayEffectExecute` clamps values, broadcasts UI delegates via the `IPawnUIInterface`, and adds `Shared::Status::Dead` when health reaches zero (`FMath::IsNearlyZero`).

### Native Gameplay Tags

All tags are declared in a centralized `WarriorRPGTags.h` using C++ namespaces that mirror the tag hierarchy. GameplayCue tags live in `DefaultGameplayTags.ini` since GAS resolves them dynamically by name.

```
WarriorRPGTags::Input::Attack::Light::Axe
WarriorRPGTags::Player::Status::TargetLocking
WarriorRPGTags::Enemy::Status::Unblockable
WarriorRPGTags::Shared::Event::SpawnProjectile
WarriorRPGTags::UI::WidgetStack::Hud
```

### Target Lock System (`UHeroGameplayAbility_TargetLock`)

Toggle ability that locks the camera and locomotion to a target enemy. Key behaviors:

- Box trace for candidate collection, selection by distance
- Screen-space widget centered over the locked target
- Per-tick update via `UAbilityTask_ExecuteTaskOnTick`
- Rotation interpolation suppressed during Rolling and Blocking states
- `SwitchTarget` uses cross product Z to classify enemies as left or right of the current target
- Dedicated IMC at priority 3 overrides look bindings during a lock session
- Guard in `EndAbility` prevents a crash during world teardown when the `PlayerController` is already invalid

### Block & Parry System

`UEnemyCombatComponent::OnHitTargetActor` evaluates incoming hits in two stages:

1. **Tag check** — is `Player::Status::Blocking` set on the defender?
2. **Geometry check** — does `IsValidBlock` (dot product, threshold −0.1f) confirm the defender is facing the attacker?

`Enemy::Status::Unblockable` on the attacking pawn bypasses both stages regardless of the defender's state, enabling specific attacks (boss finishers, telegraphed heavy strikes) to always land.

### Projectile System (`AWarriorProjectileBase`)

```
AActor
  └── AWarriorProjectileBase
        ProjectileCollisionBox (UBoxComponent, root)
        ProjectileNiagaraComponent (UNiagaraComponent)
        ProjectileMovementComp (UProjectileMovementComponent — gravity scale 0)
        ProjectileDamageEffectSpecHandle (FGameplayEffectSpecHandle, ExposeOnSpawn)
        InitialLifeSpan = 4.0f
```

`OnProjectileHit` resolves the instigator once and passes it explicitly to `HandleApplyProjectileDamage` — no re-validation inside internal calls. Block check runs against `Player::Status::Blocking` before damage application. `Shared::Event::SpawnProjectile` is sent from the ranged attack animation at the exact frame the projectile should spawn, received by the ability via `WaitGameplayEvent`.

### Enemy Summon System (`UAbilityTask_WaitSpawnEnemies`)

Custom ability task that listens for a Gameplay Event and asynchronously spawns a configurable number of `AWarriorEnemyCharacter` instances at random NavMesh-reachable locations around a given origin. The asset load is fully async via `UAssetManager`. Failed individual spawn slots are skipped with `ensureMsgf` + `continue` rather than a hard crash, and the `DidNotSpawn` delegate broadcasts if the entire spawn run produces zero valid enemies.

Each task instance consumes only its first trigger event. Spawning is server-authoritative; clients do not create local enemy actors. A valid reachable NavMesh point is required for every spawn slot.

### AI System

Enemies use behavior trees with custom C++ nodes (`BTTask`, `BTDecorator`, `BTService`). `UAIPerceptionComponent` with sight sense updates the Blackboard target via `OnEnemyPerceptionUpdated`. `UCrowdFollowingComponent` provides detour crowd avoidance. The Environment Query System determines strafing locations. `Enemy::Status::Strafing` and `Enemy::Status::UnderAttack` drive behavior tree branching.

### Function Library (`UWarriorFunctionLibrary`)

Static Blueprint Function Library centralizing shared logic:

- `IsTargetPawnHostile` — faction check via `IGenericTeamAgentInterface`, replacing naive pointer comparison
- `IsValidBlock` — dot product geometry validation (threshold −0.1f)
- `ComputeHitReactDirectionTag` — dot + cross product → `HitReact::Front/Left/Right/Back`
- `ApplyGameplayEffectSpecHandleToTargetActor` — ASC resolution with `ensureMsgf` guards
- `GetScalableFloatValueAtLevel`, `GetWidgetClassByTag`, `NativeGetWarriorASCFromActor`

---

## Project Structure

```
WarriorRPG/
├── Config/
├── Plugins/
│   ├── CommonUI/
│   ├── GameplayAbilities/
│   └── MotionWarping/
├── Source/
│   └── WarriorRPG/
│       ├── AbilitySystem/
│       │   ├── AbilityTasks/
│       │   │   ├── AbilityTask_ExecuteTaskOnTick
│       │   │   └── AbilityTask_WaitSpawnEnemies
│       │   ├── Abilities/
│       │   │   ├── Hero/
│       │   │   │   └── HeroGameplayAbility_TargetLock
│       │   │   └── Enemy/
│       │   ├── WarriorAbilitySystemComponent
│       │   ├── WarriorAttributeSet
│       │   ├── WarriorGameplayAbility
│       │   ├── WarriorHeroGameplayAbility
│       │   └── WarriorEnemyGameplayAbility
│       ├── Characters/
│       │   ├── WarriorBaseCharacter
│       │   ├── WarriorHeroCharacter
│       │   └── WarriorEnemyCharacter
│       ├── Controllers/
│       │   ├── WarriorHeroController
│       │   └── WarriorAIController
│       ├── Components/
│       │   ├── Combat/
│       │   │   ├── PawnCombatComponent
│       │   │   ├── HeroCombatComponent
│       │   │   └── EnemyCombatComponent
│       │   └── UI/
│       │       ├── PawnUIComponent
│       │       ├── HeroUIComponent
│       │       └── EnemyUIComponent
│       ├── Items/
│       │   └── Weapons/
│       │       ├── WarriorWeaponBase
│       │       └── WarriorHeroWeapon
│       ├── Projectiles/
│       │   └── WarriorProjectileBase
│       ├── UI/
│       │   ├── WarriorUISubsystem
│       │   ├── WarriorPrimaryLayout
│       │   ├── WarriorActivatableWidget
│       │   └── WarriorActionRouter
│       └── Utils/
│           ├── WarriorRPGTags
│           ├── WarriorFunctionLibrary
│           └── WarriorRPGLogCategories
└── WarriorRPG.uproject
```

---

## Getting Started

1. Clone the repository.
2. Right-click `WarriorRPG.uproject` and select **Generate Visual Studio project files** (or the equivalent for Rider).
3. Open the `.sln` or `.uproject` in your IDE and build with the **DebugGame Editor** configuration.
4. Open the project in Unreal Engine 5.6.

> This repository contains only C++ source, config files, and plugin descriptors. Binary assets are not included — the project requires the full Unreal Engine installation to compile.

---

## About

**Nicolás Bertoa** — Unreal Engine developer with 14+ years of professional experience, focused on C++ and gameplay systems.

🌐 [Portfolio](https://nbertoa.wordpress.com) | 🎬 [Demo Reels](https://nbertoa.wordpress.com/demo-reels/)
