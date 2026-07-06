#pragma once

// Gameplay enums shared across abilities, function library, and Blueprint nodes.
// Keep this file include-free — it is pulled into many headers and must stay lightweight.

#include "WarriorEnumTypes.generated.h"

/**
 * Used by BP_DoesActorHaveTag to produce Yes/No execution pins in Blueprint
 * via the ExpandEnumAsExecs meta specifier.
 */
UENUM()
enum class EWarriorConfirmType : uint8
{
	Yes,
	No
};

/**
 * Used by BP_GetPawnCombatComponentFromActor to produce Valid/Invalid execution
 * pins in Blueprint via the ExpandEnumAsExecs meta specifier.
 */
UENUM()
enum class EWarriorValidType : uint8
{
	Valid,
	Invalid
};

/**
 * Used by BP_ApplyEffectSpecHandleToTarget to produce Succeeded/Failed execution
 * pins in Blueprint via the ExpandEnumAsExecs meta specifier.
 */
UENUM()
enum class EWarriorSuccessType : uint8
{
	Succeeded,
	Failed
};

/**
 * Identifies which weapon slot should have its collision toggled.
 * Used by UPawnCombatComponent::ToggleWeaponCollision and Animation Notifies
 * to select between the currently equipped weapon and unarmed hand slots.
 */
UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	/** The weapon currently in the equipped slot (most common case). */
	CurrentEquippedWeapon,
	/** Unarmed left hand — for punch/kick attacks without a weapon actor. */
	LeftHand,
	/** Unarmed right hand — for punch/kick attacks without a weapon actor. */
	RightHand
};
