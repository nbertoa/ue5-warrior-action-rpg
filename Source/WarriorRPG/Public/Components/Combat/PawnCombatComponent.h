// PawnCombatComponent.h
// Base combat component shared by all characters in WarriorRPG (hero and enemies).
// Manages the weapon registry and equipped weapon state. Subclasses add
// faction-specific combat behavior (hero combos, enemy AI telegraphs).

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnExtensionComponentBase.h"
#include "GameplayTagContainer.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "PawnCombatComponent.generated.h"

class AWarriorWeaponBase;

/**
 * Base combat component for WarriorRPG Pawns.
 * Provides the foundation for weapon management, hit detection, and combat state
 * that both hero and enemy characters share.
 *
 * Weapons are indexed by Gameplay Tag so the same pawn can carry multiple weapons
 * simultaneously (e.g., a sword and a shield) and switch between them by tag.
 * Only one weapon is "equipped" at a time — the one combat abilities resolve
 * attacks through.
 *
 * Subclass this component when adding a new combat-capable character type
 * rather than creating a parallel hierarchy — this keeps all combat
 * functionality consolidated and avoids duplicating weapon registry logic.
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()

public:
	/**
	 * Registers a spawned weapon in this pawn's weapon registry under the given tag.
	 * Binds the weapon's hit delegates to this component's OnHitTargetActor and
	 * OnWeaponPulledFromTargetActor callbacks. Optionally marks the weapon as
	 * currently equipped.
	 *
	 * @param InWeaponTag      The tag identifying this weapon (e.g., Player::Weapon::Axe).
	 * @param InWeapon         The spawned weapon actor to register. Must not be null.
	 * @param bEquippedWeapon  If true, sets this weapon as the current equipped weapon
	 *                         by updating CurrentEquippedWeaponTag.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	void RegisterSpawnedWeapon(FGameplayTag InWeaponTag,
	                           AWarriorWeaponBase* InWeapon,
	                           bool bEquippedWeapon = false);

	/**
	 * Looks up a previously registered weapon by its tag.
	 * Logs a warning if the tag is not found in the registry.
	 *
	 * @param InWeaponTag  The tag of the weapon to retrieve.
	 * @return             The weapon associated with the tag, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCarriedWeaponByTag(FGameplayTag InWeaponTag) const;

	/**
	 * Returns the currently equipped weapon, or nullptr if no weapon is equipped
	 * or if CurrentEquippedWeaponTag is invalid.
	 * Delegates the lookup to GetCarriedWeaponByTag.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCurrentEquippedWeapon() const;

	/**
	 * Enables or disables the collision box on the specified weapon slot.
	 * Called from Animation Notifies to open and close hit detection windows
	 * during attack animations.
	 * Clears OverlappedActors when disabling so the next swing starts fresh.
	 *
	 * @param bShouldEnable     True to set QueryOnly collision, false to disable it.
	 * @param ToggleDamageType  Which weapon slot to toggle. Defaults to the current
	 *                          equipped weapon; pass LeftHand or RightHand for
	 *                          unarmed/body-collision attacks.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	void ToggleWeaponCollision(bool bShouldEnable,
	                           EToggleDamageType ToggleDamageType = EToggleDamageType::CurrentEquippedWeapon);

	/**
	 * Tag identifying the currently equipped weapon.
	 * An invalid tag means no weapon is currently equipped.
	 * Updated automatically by RegisterSpawnedWeapon when bEquippedWeapon is true,
	 * and by equip/unequip abilities when the active weapon changes at runtime.
	 */
	UPROPERTY(BlueprintReadWrite,
		Category = "Warrior|Combat")
	FGameplayTag CurrentEquippedWeaponTag;

	/**
	 * Called by the weapon's hit delegate when its collision box begins overlapping
	 * a new target pawn during an attack swing.
	 * Override in subclasses to implement faction-specific hit reactions:
	 * hero sends MeleeHit to self; enemy evaluates block then sends MeleeHit or SuccessfulBlock.
	 *
	 * @param HitActor  The actor that was hit by the weapon collision box.
	 */
	virtual void OnHitTargetActor(AActor* HitActor);

	/**
	 * Called by the weapon's pull delegate when its collision box stops overlapping
	 * a target pawn. Override to clean up per-hit state if needed.
	 *
	 * @param InteractedActor  The actor the weapon just stopped overlapping.
	 */
	virtual void OnWeaponPulledFromTargetActor(AActor* InteractedActor);

protected:
	/**
	 * Toggles collision on the weapon actor currently stored under CurrentEquippedWeaponTag.
	 * Called by ToggleWeaponCollision when ToggleDamageType is CurrentEquippedWeapon.
	 *
	 * @param bShouldEnable  True to set QueryOnly collision, false to disable it.
	 */
	virtual void ToggleCurrentEquippedWeaponCollision(bool bShouldEnable);

	/**
	 * Toggles collision on one of the pawn's body collision boxes (left or right hand).
	 * Called by ToggleWeaponCollision when ToggleDamageType is LeftHand or RightHand.
	 * Override in subclasses that own body collision boxes (e.g., UEnemyCombatComponent).
	 *
	 * @param bShouldEnable    True to set QueryOnly collision, false to disable it.
	 * @param ToggleDamageType Which hand box to toggle (LeftHand or RightHand).
	 */
	virtual void ToggleBodyCollisionBoxCollision(bool bShouldEnable,
	                                             EToggleDamageType ToggleDamageType);

	/**
	 * Tracks which actors have already been hit during the current attack swing.
	 * Populated in OnHitTargetActor and cleared when weapon collision is disabled.
	 * Prevents the same actor from being hit multiple times in a single swing.
	 */
	TArray<AActor*> OverlappedActors;

private:
	/**
	 * Map of all weapons this pawn is currently carrying, indexed by Gameplay Tag.
	 * A pawn can carry multiple weapons simultaneously; only one is "equipped" at a time.
	 * Private to enforce registration exclusively through RegisterSpawnedWeapon,
	 * which validates inputs and binds hit delegates before storing the weapon.
	 */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<AWarriorWeaponBase>> CarriedWeaponByTag;
};
