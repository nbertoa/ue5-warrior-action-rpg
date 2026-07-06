// WarriorRPGTags.cpp
// Defines and registers all native Gameplay Tags declared in WarriorRPGTags.h.

#include "Utils/WarriorRPGTags.h"

namespace WarriorRPGTags::Input
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Move,
	                               "WarriorRPGTags.Input.Move",
	                               "Tag for movement input (WASD / left stick).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Look,
	                               "WarriorRPGTags.Input.Look",
	                               "Tag for camera look input (mouse delta / right stick).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(EquipAxe,
	                               "WarriorRPGTags.Input.EquipAxe",
	                               "Tag for the Equip Axe gameplay ability input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UnequipAxe,
	                               "WarriorRPGTags.Input.UnequipAxe",
	                               "Tag for the Unequip Axe gameplay ability input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Roll,
	                               "WarriorRPGTags.Input.Roll",
	                               "Tag for the Roll gameplay ability input.");

	// Bound to both Triggered and Completed — Triggered captures direction each frame,
	// Completed fires the switch event once when the gesture ends.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SwitchTarget,
	                               "WarriorRPGTags.Input.SwitchTarget",
	                               "Input tag for the target switch gesture (right stick flick / mouse swipe).");
}

namespace WarriorRPGTags::Input::MustBeHeld
{
	// Parent tag checked via MatchesTag in OnGameplayAbilityInputReleased
	// to identify inputs that cancel their ability on button release.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag,
	                               "WarriorRPGTags.Input.MustBeHeld",
	                               "Parent hierarchy tag for held-input abilities — matched via MatchesTag on release.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Block,
	                               "WarriorRPGTags.Input.MustBeHeld.Block",
	                               "Held input tag for the block ability — ability cancels when button is released.");
}

namespace WarriorRPGTags::Input::Toggleable
{
	// Parent tag checked via MatchesTag in OnGameplayAbilityInputPressed
	// to identify inputs that toggle their ability on each press.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Tag,
	                               "WarriorRPGTags.Input.Toggleable",
	                               "Parent hierarchy tag for toggle-style inputs — ability activates or cancels on each press.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TargetLock,
	                               "WarriorRPGTags.Input.Toggleable.TargetLock",
	                               "Toggle input tag for the target lock ability.");
}

namespace WarriorRPGTags::Input::Attack::Light
{
	// Ability input tag — forwarded to the ASC on press, not bound to C++ directly.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Input.Attack.Light.Axe",
	                               "Tag for the axe light attack input.");
}

namespace WarriorRPGTags::Input::Attack::Heavy
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Input.Attack.Heavy.Axe",
	                               "Tag for the axe heavy attack input.");
}

namespace WarriorRPGTags::Player::Ability
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitPause,
	                               "WarriorRPGTags.Player.Ability.HitPause",
	                               "Identity tag for the hero hit pause ability.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Roll,
	                               "WarriorRPGTags.Player.Ability.Roll",
	                               "Identity tag for the hero roll gameplay ability.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Block,
	                               "WarriorRPGTags.Player.Ability.Block",
	                               "Identity tag for the hero block gameplay ability.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TargetLock,
	                               "WarriorRPGTags.Player.Ability.TargetLock",
	                               "Identity tag for the hero target lock gameplay ability.");
}

namespace WarriorRPGTags::Player::Ability::Equip
{
	// Identity tag of UGA_Hero_EquipAxe — used for blocking, cancellation, and cooldowns.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Ability.Equip.Axe",
	                               "Identity tag for the Equip Axe gameplay ability.");
}

namespace WarriorRPGTags::Player::Ability::Unequip
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Ability.Unequip.Axe",
	                               "Identity tag for the Unequip Axe gameplay ability.");
}

namespace WarriorRPGTags::Player::Ability::Attack::Light
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Ability.Attack.Light.Axe",
	                               "Identity tag for the axe light attack gameplay ability.");
}

namespace WarriorRPGTags::Player::Ability::Attack::Heavy
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Ability.Attack.Heavy.Axe",
	                               "Identity tag for the axe heavy attack gameplay ability.");
}

namespace WarriorRPGTags::Player::Event
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitPause,
	                               "WarriorRPGTags.Player.Event.HitPause",
	                               "Gameplay Event sent to the hero to trigger the hit pause effect.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SuccessfulBlock,
	                               "WarriorRPGTags.Player.Event.SuccessfulBlock",
	                               "Gameplay Event sent to the defending hero when a block geometry check passes.");
}

namespace WarriorRPGTags::Player::Event::Equip
{
	// Sent by an AnimNotify at the attach frame — the equip ability waits for this
	// via WaitGameplayEvent to know when to move the weapon to the hand socket.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Event.Equip.Axe",
	                               "Gameplay Event sent at the equip attach point in the axe equip animation.");
}

namespace WarriorRPGTags::Player::Event::Unequip
{
	// Sent by an AnimNotify at the detach frame — the unequip ability waits for this
	// to move the weapon back to the holster socket.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Event.Unequip.Axe",
	                               "Gameplay Event sent at the detach point in the axe unequip animation.");
}

namespace WarriorRPGTags::Player::Event::SwitchTarget
{
	// Fired by Input_SwitchTargetCompleted when the gesture ends with a leftward direction.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Left,
	                               "WarriorRPGTags.Player.Event.SwitchTarget.Left",
	                               "Gameplay Event sent to the target lock ability to switch to the nearest enemy on the left.")
	;

	// Fired by Input_SwitchTargetCompleted when the gesture ends with a rightward direction.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Right,
	                               "WarriorRPGTags.Player.Event.SwitchTarget.Right",
	                               "Gameplay Event sent to the target lock ability to switch to the nearest enemy on the right.")
	;
}

namespace WarriorRPGTags::Player::SetByCaller::AttackType
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Light,
	                               "WarriorRPGTags.Player.SetByCaller.AttackType.Light",
	                               "SetByCaller tag used to pass light attack combo count into Gameplay Effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Heavy,
	                               "WarriorRPGTags.Player.SetByCaller.AttackType.Heavy",
	                               "SetByCaller tag used to pass heavy attack combo count into Gameplay Effects.");
}

namespace WarriorRPGTags::Player::Status
{
	// Transient state tag — set when the heavy attack combo reaches the finisher window.
	// Not tied to any ability or effect; managed directly via loose tag API.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(JumpToFinisher,
	                               "WarriorRPGTags.Player.Status.JumpToFinisher",
	                               "Status tag active when the heavy attack combo finisher is available.");

	// Transient state tag — set while the hero roll ability is actively executing.
	// Used to block conflicting abilities and drive animation state.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Rolling,
	                               "WarriorRPGTags.Player.Status.Rolling",
	                               "Status tag active while the hero is executing a roll.");

	// Transient state tag — set while the hero block ability is actively executing.
	// Checked by UEnemyCombatComponent::OnHitTargetActor before the geometry validation.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Blocking,
	                               "WarriorRPGTags.Player.Status.Blocking",
	                               "Status tag active while the hero block ability is executing.");

	// Transient state tag — set while the hero target lock ability is actively executing.
	// Used to drive camera behavior, locomotion orientation, and UI indicator visibility.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TargetLocking,
	                               "WarriorRPGTags.Player.Status.TargetLocking",
	                               "Status tag active while the hero target lock ability is executing.");
}

namespace WarriorRPGTags::Player::Weapon
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Axe,
	                               "WarriorRPGTags.Player.Weapon.Axe",
	                               "Tag for the player's axe weapon.");
}

namespace WarriorRPGTags::Enemy
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Weapon,
	                               "WarriorRPGTags.Enemy.Weapon",
	                               "Tag for the enemy's weapon.");
}

namespace WarriorRPGTags::Enemy::Ability
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Melee,
	                               "WarriorRPGTags.Enemy.Ability.Melee",
	                               "Identity tag for enemy melee combat abilities.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ranged,
	                               "WarriorRPGTags.Enemy.Ability.Ranged",
	                               "Identity tag for enemy ranged combat abilities.");

	// Identity tag of the enemy summon ability — used for blocking, cancellation, and cooldowns.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SummonEnemies,
	                               "WarriorRPGTags.Enemy.Ability.SummonEnemies",
	                               "Identity tag for the enemy summon ability — used for "
	                               "blocking, cancellation, and cooldown queries in GAS.");
}

namespace WarriorRPGTags::Enemy::Event
{
	// Sent by an AnimNotify or ability at the frame when summoning should begin —
	// the summon ability listens via WaitGameplayEvent to know the exact spawn moment.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SummonEnemies,
	                               "WarriorRPGTags.Enemy.Event.SummonEnemies",
	                               "Gameplay Event sent to trigger the enemy summon sequence "
	                               "at the correct animation frame.");
}

namespace WarriorRPGTags::Enemy::Status
{
	// Transient state tag — set while the enemy is actively strafing around its target.
	// Not tied to any ability or effect; managed directly via loose tag API.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Strafing,
	                               "WarriorRPGTags.Enemy.Status.Strafing",
	                               "Status tag active while an enemy is strafing around its target.");

	// Transient state tag — set when the enemy is actively being attacked by the player.
	// Used by the AI behavior tree to interrupt or modify current behavior.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UnderAttack,
	                               "WarriorRPGTags.Enemy.Status.UnderAttack",
	                               "Status tag active while an enemy is being attacked by the player.");

	// Transient state tag — set on an attacking enemy pawn to mark its current attack
	// as ignoring the player's block state. Checked by UEnemyCombatComponent::OnHitTargetActor
	// before the geometry validation, bypassing IsValidBlock entirely when present.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Unblockable,
	                               "WarriorRPGTags.Enemy.Status.Unblockable",
	                               "Status tag set on an enemy pawn while its current attack ignores "
	                               "the player's block state — checked by UEnemyCombatComponent::OnHitTargetActor.");
}

namespace WarriorRPGTags::Shared::Ability
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact,
	                               "WarriorRPGTags.Shared.Ability.HitReact",
	                               "Identity tag for the hit react gameplay ability shared by hero and enemies.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Death,
	                               "WarriorRPGTags.Shared.Ability.Death",
	                               "Identity tag for the death gameplay ability shared by hero and enemies.");
}

namespace WarriorRPGTags::Shared::Event
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MeleeHit,
	                               "WarriorRPGTags.Shared.Event.MeleeHit",
	                               "Gameplay Event sent to the hero when the weapon collision box hits a valid target.")
	;

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact,
	                               "WarriorRPGTags.Shared.Event.HitReact",
	                               "Gameplay Event sent to a character when it takes a hit to trigger the hit react ability.")
	;

	// Sent from a ranged attack ability's animation at the spawn frame — the receiving
	// ability listens via WaitGameplayEvent to know exactly when to spawn the projectile.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SpawnProjectile,
	                               "WarriorRPGTags.Shared.Event.SpawnProjectile",
	                               "Gameplay Event sent to trigger projectile spawning from a ranged attack ability.");
}

namespace WarriorRPGTags::Shared::SetByCaller
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BaseDamage,
	                               "WarriorRPGTags.Shared.SetByCaller.BaseDamage",
	                               "SetByCaller tag used to pass base damage magnitude into Gameplay Effects at application time.")
	;
}

namespace WarriorRPGTags::Shared::Status
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Dead,
	                               "WarriorRPGTags.Shared.Status.Dead",
	                               "Loose status tag added when a character's CurrentHealth reaches zero.");
}

namespace WarriorRPGTags::Shared::Status::HitReact
{
	// Directional tags are added by UWarriorAttributeSet::PostGameplayEffectExecute
	// via ComputeHitReactDirectionTag() to parameterize the hit react animation.

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Front,
	                               "WarriorRPGTags.Shared.Status.HitReact.Front",
	                               "Hit react direction — attacker is in front of the victim (within ±45°).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Left,
	                               "WarriorRPGTags.Shared.Status.HitReact.Left",
	                               "Hit react direction — attacker is to the left of the victim (-135° to -45°).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Right,
	                               "WarriorRPGTags.Shared.Status.HitReact.Right",
	                               "Hit react direction — attacker is to the right of the victim (45° to 135°).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Back,
	                               "WarriorRPGTags.Shared.Status.HitReact.Back",
	                               "Hit react direction — attacker is behind the victim (beyond ±135°).");
}

namespace WarriorRPGTags::UI::WidgetStack
{
	// Stack tags identify which layer a widget should be pushed to in UWarriorPrimaryLayout.
	// UWarriorUISubsystem::PushWidgetToStack uses these tags to find the correct container.

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Background,
	                               "WarriorRPGTags.UI.WidgetStack.Background",
	                               "Lowest z-order stack — full-screen backgrounds and loading screens.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Hud,
	                               "WarriorRPGTags.UI.WidgetStack.Hud",
	                               "HUD stack — health bars, ability indicators, minimap.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Modal,
	                               "WarriorRPGTags.UI.WidgetStack.Modal",
	                               "Modal stack — dialogs and menus that require exclusive player attention.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Toasts,
	                               "WarriorRPGTags.UI.WidgetStack.Toasts",
	                               "Primary toast stack — transient non-blocking notifications.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Toasts2,
	                               "WarriorRPGTags.UI.WidgetStack.Toasts2",
	                               "Secondary toast stack — second independent notification stream.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Toasts3,
	                               "WarriorRPGTags.UI.WidgetStack.Toasts3",
	                               "Tertiary toast stack — third independent notification stream.");
}
