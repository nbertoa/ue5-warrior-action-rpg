// WarriorRPGTags.h
// Centralized declaration of all native Gameplay Tags for the WarriorRPG project.
// Native tags are registered at module startup — no ini files or DataTables needed.
// Organized by system using nested namespaces (Input, Player, Enemy, Combat, UI…).

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace WarriorRPGTags::Input
{
    /** Tag for movement input (WASD / left stick). Bound to a native C++ handler. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Move);

    /** Tag for camera look input (mouse delta / right stick). Bound to a native C++ handler. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Look);

    /** Tag for the Equip Axe gameplay ability input. Activates UGA_Hero_EquipAxe via GAS. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(EquipAxe);

    /** Tag for the Unequip Axe gameplay ability input. Activates UGA_Hero_UnequipAxe via GAS. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(UnequipAxe);

    /** Tag for the Roll gameplay ability input. Activates UGA_Hero_Roll via GAS. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Roll);

    /**
     * Input tag for the target switch gesture (right stick flick / mouse swipe).
     * Bound to both Triggered and Completed events on the hero character:
     * Triggered captures the direction each frame; Completed fires the switch event once.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(SwitchTarget);
}

namespace WarriorRPGTags::Input::MustBeHeld
{
    /**
     * Parent hierarchy tag for all inputs that must be held to keep their ability active.
     * OnGameplayAbilityInputReleased checks MatchesTag against this to decide
     * whether to cancel the ability when the button is released.
     * Any input tag under this namespace automatically inherits that behavior.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag);

    /**
     * Input tag for the block ability. Held-input variant — the block ability
     * stays active as long as this button is held and cancels on release.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Block);
}

namespace WarriorRPGTags::Input::Toggleable
{
    /**
     * Parent hierarchy tag for all toggle-style inputs.
     * OnGameplayAbilityInputPressed checks MatchesTag against this to decide
     * whether to toggle (cancel if active, activate if inactive) instead of
     * always activating on press.
     * Any input tag under this namespace automatically inherits that behavior.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag);

    /**
     * Toggle input tag for the target lock ability.
     * First press activates the lock; second press cancels it.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TargetLock);

    /** Toggle input tag for the rage ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rage);
}

namespace WarriorRPGTags::Input::Attack::Light
{
    /** Tag for the axe light attack input. Activates the light attack ability via GAS. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Input::Attack::Heavy
{
    /** Tag for the axe heavy attack input. Activates the heavy attack ability via GAS. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Input::SpecialWeaponAbility
{
    /** Input tag for the light special weapon ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Light);

    /** Input tag for the heavy special weapon ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heavy);
}

namespace WarriorRPGTags::Player::Ability
{
    /**
     * Identity tag for the hero hit pause ability (UGA_Hero_HitPause).
     * Activated via WaitGameplayEvent(Player::Event::HitPause) when a weapon hit registers,
     * briefly freezing time to add impact feedback to melee attacks.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitPause);

    /**
     * Identity tag for the hero roll gameplay ability (UGA_Hero_Roll).
     * Used for blocking tags, cancellation queries, and cooldown tracking.
     * Distinct from Input::Roll (the button) and Player::Status::Rolling (the active state).
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Roll);

    /**
     * Identity tag for the hero block gameplay ability (UGA_Hero_Block).
     * Used for blocking tags, cancellation queries, and cooldown tracking.
     * Distinct from Input::MustBeHeld::Block (the button) and Player::Status::Blocking (the active state).
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Block);

    /**
     * Identity tag for the hero target lock gameplay ability (UHeroGameplayAbility_TargetLock).
     * Used for blocking tags, cancellation queries, and cooldown tracking.
     * Distinct from Input::Toggleable::TargetLock (the button) and Player::Status::TargetLocking (the active state).
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TargetLock);

    /**
     * Identity tag for the hero rage gameplay ability.
     * Distinct from Input::Toggleable::Rage, which represents the input binding.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rage);
}

namespace WarriorRPGTags::Player::Ability::Equip
{
    /**
     * Identity tag for the Equip Axe gameplay ability (UGA_Hero_EquipAxe).
     * Used for blocking tags, cancellation queries, and cooldown tracking.
     * Distinct from Input::EquipAxe (the button) and Event::Equip::Axe (the anim notify).
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Ability::Unequip
{
    /**
     * Identity tag for the Unequip Axe gameplay ability (UGA_Hero_UnequipAxe).
     * Used for blocking tags, cancellation queries, and cooldown tracking.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Ability::Attack::Light
{
    /**
     * Identity tag for the axe light attack gameplay ability.
     * Used for blocking, cancellation, and cooldown queries in GAS.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Ability::Attack::Heavy
{
    /**
     * Identity tag for the axe heavy attack gameplay ability.
     * Used for blocking, cancellation, and cooldown queries in GAS.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Ability::SpecialWeaponAbility
{
    /** Identity tag for the light special weapon gameplay ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Light);

    /** Identity tag for the heavy special weapon gameplay ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heavy);
}

namespace WarriorRPGTags::Player::Cooldown::SpecialWeaponAbility
{
    /** Cooldown tag for the light special weapon ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Light);

    /** Cooldown tag for the heavy special weapon ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heavy);
}

namespace WarriorRPGTags::Player::Event
{
    /**
     * Gameplay Event sent to the hero when a weapon hit registers or a weapon separates from a target.
     * Received by UGA_Hero_HitPause via WaitGameplayEvent to trigger the hit pause effect.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitPause);

    /**
     * Gameplay Event sent to the hero defender when a block geometry check passes.
     * Received by the block ability via WaitGameplayEvent to trigger the block reaction
     * and suppress the incoming damage application.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(SuccessfulBlock);

    /** Gameplay Event sent to request activation of the hero rage ability. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateRage);
}

namespace WarriorRPGTags::Player::Event::Equip
{
    /**
     * Gameplay Event sent by an AnimNotify at the frame when the axe should attach
     * to the hero's hand socket during the equip animation.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Event::Unequip
{
    /**
     * Gameplay Event sent by an AnimNotify at the frame when the axe should detach
     * from the hand socket and return to the holster during the unequip animation.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Player::Event::SwitchTarget
{
    /**
     * Gameplay Event sent to the target lock ability when the player flicks left
     * to switch the lock to the nearest enemy on the left side of the current target.
     * Fired by AWarriorHeroCharacter::Input_SwitchTargetCompleted on gesture end.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Left);

    /**
     * Gameplay Event sent to the target lock ability when the player flicks right
     * to switch the lock to the nearest enemy on the right side of the current target.
     * Fired by AWarriorHeroCharacter::Input_SwitchTargetCompleted on gesture end.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Right);
}

namespace WarriorRPGTags::Player::SetByCaller::AttackType
{
    /** SetByCaller tag used to pass light attack combo count into Gameplay Effects.
     *  Passed as the magnitude for the attack type tag in MakeHeroDamageEffectSpecHandle. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Light);

    /** SetByCaller tag used to pass heavy attack combo count into Gameplay Effects.
     *  Passed as the magnitude for the attack type tag in MakeHeroDamageEffectSpecHandle. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heavy);
}

namespace WarriorRPGTags::Player::Status
{
    /**
     * Loose status tag set during the heavy attack combo when the finisher move
     * becomes available. Checked by the combo system to decide whether to branch
     * into the finisher or continue the standard combo chain.
     * Added and removed via UWarriorFunctionLibrary::AddGameplayTagToActor/RemoveGameplayTagFromActor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(JumpToFinisher);

    /**
     * Loose status tag set on the hero while a roll ability is actively executing.
     * Used to block conflicting abilities (attacks, equip) and drive animation state.
     * Added at roll start and removed when the roll ability ends.
     * Added and removed via UWarriorFunctionLibrary::AddGameplayTagToActor/RemoveGameplayTagFromActor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rolling);

    /**
     * Loose status tag set on the hero while the block ability is actively executing.
     * Checked by UEnemyCombatComponent::OnHitTargetActor to determine whether the
     * incoming hit should be intercepted and routed through the block validation path.
     * Added at block start and removed when the block ability ends or is cancelled.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Blocking);

    /**
     * Loose status tag set on the hero while the target lock ability is actively executing.
     * Used to drive camera behavior, locomotion orientation, and UI indicator visibility.
     * Added when target lock activates and removed when it ends or is cancelled.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TargetLocking);
}

namespace WarriorRPGTags::Player::Status::Rage
{
    /** Rage lifecycle statuses; Full and None are synchronized by the AttributeSet. */

    /** Rage is entering its active state. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Activating);

    /** Rage is currently active. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Active);

    /** The rage resource has reached its maximum value. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Full);

    /** The rage resource is empty. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(None);
}

namespace WarriorRPGTags::Player::Weapon
{
    /** Tag identifying the player's axe in the combat component's weapon registry. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Axe);
}

namespace WarriorRPGTags::Enemy
{
    /** Tag identifying the enemy's weapon in the combat component's weapon registry.
     *  Mirrors Player::Weapon::Axe in purpose — used as the key in
     *  UPawnCombatComponent::CarriedWeaponByTag for enemy weapon lookup. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon);
}

namespace WarriorRPGTags::Enemy::Ability
{
    /** Identity tag for enemy melee combat abilities.
     *  Used for blocking, cancellation, and cooldown queries in GAS for AI-driven melee attacks. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Melee);

    /** Identity tag for enemy ranged combat abilities.
     *  Used for blocking, cancellation, and cooldown queries in GAS for AI-driven ranged attacks. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ranged);

    /**
     * Identity tag for the enemy summon ability.
     * Used for blocking, cancellation, and cooldown queries in GAS
     * for AI-driven enemy summoning behavior.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(SummonEnemies);
}

namespace WarriorRPGTags::Enemy::Event
{
    /**
     * Gameplay Event sent to trigger the enemy summon sequence.
     * Received via WaitGameplayEvent inside the summon ability
     * at the exact animation frame when summoning should occur.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(SummonEnemies);
}

namespace WarriorRPGTags::Enemy::Status
{
    /**
     * Loose status tag set on an enemy while it is strafing around its target.
     * Used by the AI behavior tree and animation system to drive strafe locomotion.
     * Added and removed via UWarriorFunctionLibrary::AddGameplayTagToActor/RemoveGameplayTagFromActor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Strafing);

    /**
     * Loose status tag set on an enemy when it is actively being attacked by the player.
     * Used by the AI behavior tree to interrupt or modify current behavior
     * (e.g., break out of idle patrol, trigger a defensive response).
     * Added and removed via UWarriorFunctionLibrary::AddGameplayTagToActor/RemoveGameplayTagFromActor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(UnderAttack);

    /**
     * Loose status tag set on an enemy pawn while its current attack ignores
     * the player's block state, regardless of the defender's facing.
     * Checked by UEnemyCombatComponent::OnHitTargetActor before the geometry validation —
     * when present, the block check is skipped entirely and the hit always lands as MeleeHit.
     * Added and removed via UWarriorFunctionLibrary::AddGameplayTagToActor/RemoveGameplayTagFromActor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Unblockable);
}

namespace WarriorRPGTags::Shared::Ability
{
    /** Identity tag for the hit react gameplay ability shared by hero and enemies.
     *  Granted at startup and activated via WaitGameplayEvent(Shared::Event::HitReact)
     *  when a character receives a hit. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);

    /**
     * Identity tag for the death gameplay ability shared by hero and enemies.
     * Activated when Shared::Status::Dead is added to the character's ASC,
     * triggering the death animation and any post-death cleanup logic.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Death);
}

namespace WarriorRPGTags::Shared::Event
{
    /** Gameplay Event sent to the hero when the weapon collision box hits a valid target.
     *  Received by attack abilities via WaitGameplayEvent to trigger damage application. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeleeHit);

    /** Gameplay Event sent to a character when it takes a hit.
     *  Received by the hit react ability via WaitGameplayEvent to trigger the hit react animation. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);

    /**
     * Gameplay Event sent to trigger projectile spawning from a ranged attack ability.
     * Received via WaitGameplayEvent inside the enemy/hero ranged attack ability
     * to know the exact animation frame at which to spawn the projectile actor.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(SpawnProjectile);
}

namespace WarriorRPGTags::Shared::SetByCaller
{
    /** SetByCaller tag used to pass base damage magnitude into Gameplay Effects at application time.
     *  The caller sets this value on the EffectSpec before applying the effect —
     *  the GE reads it instead of using a hard-coded magnitude. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(BaseDamage);
}

namespace WarriorRPGTags::Shared::Status
{
    /**
     * Loose status tag added to a character's ASC when its CurrentHealth reaches zero.
     * Presence of this tag signals that the character is dead — used to block further
     * ability activation and to activate the death ability via tag-based triggering.
     * Added via UWarriorFunctionLibrary::AddGameplayTagToActor in PostGameplayEffectExecute.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);

    /** Status tag set while a character is immune to incoming damage. */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Invincible);
}

namespace WarriorRPGTags::Shared::Status::HitReact
{
    /**
     * Directional hit react tag set when the attacker is in front of the victim.
     * Angle range: [-45°, +45°] relative to the victim's forward vector.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Front);

    /**
     * Directional hit react tag set when the attacker is to the left of the victim.
     * Angle range: (-135°, -45°) relative to the victim's forward vector.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Left);

    /**
     * Directional hit react tag set when the attacker is to the right of the victim.
     * Angle range: (45°, 135°] relative to the victim's forward vector.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Right);

    /**
     * Directional hit react tag set when the attacker is behind the victim.
     * Angle range: < -135° or > 135° relative to the victim's forward vector.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Back);
}

namespace WarriorRPGTags::UI::WidgetStack
{
    /**
     * Tag identifying the background widget stack layer.
     * Used as the lowest z-order layer — full-screen backgrounds, loading screens,
     * and any content that should always appear behind all other UI.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Background);

    /**
     * Tag identifying the HUD widget stack layer.
     * Contains gameplay HUD elements: health bars, ability indicators, minimap.
     * Sits above the background but below modal and toast layers.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Hud);

    /**
     * Tag identifying the modal widget stack layer.
     * Used for dialogs, menus, and any overlay that requires exclusive player attention.
     * Input routing blocks underlying layers while a modal widget is active.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Modal);

    /**
     * Tag identifying the primary toast notification stack layer.
     * Used for transient non-blocking notifications (item pickups, quest updates).
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Toasts);

    /**
     * Tag identifying the secondary toast stack layer.
     * Used when two independent toast streams need to coexist simultaneously
     * without interfering with each other's queue.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Toasts2);

    /**
     * Tag identifying the tertiary toast stack layer.
     * Reserved for a third independent notification stream when the primary
     * and secondary toast stacks are already in use.
     */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Toasts3);
}
