// WarriorHeroWeapon.h
// Hero-specific weapon class.
// Extends the weapon base with player-facing data: the Linked Anim Layer,
// weapon IMC, weapon abilities, and the spec handles for those granted abilities.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorHeroWeapon.generated.h"

/**
 * Weapon class for the hero character.
 * Inherits mesh and collision setup from AWarriorWeaponBase.
 *
 * Owns the granted ability spec handles for its weapon abilities.
 * On equip: the equip ability stores handles via AssignGrantedAbilitySpecHandles.
 * On unequip: the unequip ability retrieves them via GetGrantedAbilitySpecHandles
 *             and passes them to UWarriorAbilitySystemComponent::RemoveGrantedHeroWeaponAbilities.
 * On EndPlay: remaining handles are force-revoked as a failsafe against orphaned specs.
 */
UCLASS()
class WARRIORRPG_API AWarriorHeroWeapon : public AWarriorWeaponBase
{
	GENERATED_BODY()

public:
	/**
	 * Hero-specific configuration data for this weapon.
	 * Defines the anim layer, input mapping context, and weapon abilities.
	 * Configure all fields in the weapon's Blueprint defaults.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Settings")
	FWarriorHeroWeaponData HeroWeaponData;

	/**
	 * Stores the spec handles returned by GrantHeroWeaponAbilities.
	 * Called by the equip ability immediately after granting weapon abilities.
	 * Must only be called once per equip — calling again without clearing would
	 * overwrite existing handles and prevent proper cleanup on unequip.
	 *
	 * @param InSpecHandles     Handles returned by GrantHeroWeaponAbilities. Must not be empty.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Weapon")
	void AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles);

	/**
	 * Returns the stored ability spec handles for this weapon's granted abilities.
	 * Called by the unequip ability to retrieve handles for revocation.
	 * Returns by const reference to avoid copying the array.
	 *
	 * @return  Handles assigned via AssignGrantedAbilitySpecHandles.
	 *          Empty if the weapon has never been equipped or abilities were already cleared.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Weapon")
	const TArray<FGameplayAbilitySpecHandle>& GetGrantedAbilitySpecHandles() const;

protected:
	//~ Begin AActor Interface
	/**
	 * Failsafe cleanup triggered when the weapon is removed from the world.
	 * If GrantedAbilitySpecHandles is not empty at this point, the normal unequip
	 * path was skipped (character death, level unload, etc.) — abilities are force-revoked
	 * from the ASC to prevent orphaned specs that can never be activated or cleaned up.
	 *
	 * Note: Super::EndPlay is called AFTER cleanup so the weapon is still in a valid
	 * state when we access its owner and components.
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

private:
	/** Cached handles for abilities granted when this weapon was last equipped. */
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
};
