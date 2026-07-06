// WarriorAbilitySystemComponent.h
// Project-specific Ability System Component for WarriorRPG.
// Extends the engine's ASC with input-driven ability activation and weapon ability granting.
// All characters in WarriorRPG use this component, ensuring consistent ability behavior.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "WarriorAbilitySystemComponent.generated.h"

/**
 * Custom Ability System Component for WarriorRPG.
 * Provides two extensions over the base UAbilitySystemComponent:
 *
 * 1. Tag-based input routing: instead of GAS's numeric InputID system, abilities are
 *    identified by Gameplay Tags injected into their spec's DynamicSpecSourceTags at
 *    grant time. Input events forward the tag to OnGameplayAbilityInputPressed/Released.
 *
 * 2. Weapon ability lifecycle: GrantHeroWeaponAbilities grants a weapon's ability set
 *    and returns handles; RemoveGrantedHeroWeaponAbilities revokes them on unequip.
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UWarriorAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	 * Called when a gameplay ability input button is pressed.
	 * Searches all activatable specs for one whose DynamicSpecSourceTags contain
	 * InInputTag exactly, then calls TryActivateAbility on that spec's handle.
	 *
	 * Uses HasTagExact (not HasTag) to prevent partial hierarchy matches.
	 *
	 * @param InInputTag    The input tag forwarded from the character's input handler.
	 */
	void OnGameplayAbilityInputPressed(const FGameplayTag& InInputTag);

	/**
	 * Called when a gameplay ability input button is released.
	 * Currently a stub — will be implemented when the first hold-to-charge or
	 * toggle ability is introduced.
	 *
	 * @param InInputTag    The input tag identifying which ability was released.
	 */
	void OnGameplayAbilityInputReleased(const FGameplayTag& InInputTag);

	/**
	 * Grants all abilities in InDefaultWeaponAbilities to this ASC.
	 * Each ability's InputTag is injected into DynamicSpecSourceTags for input routing.
	 * Called by UGA_Hero_EquipAxe — the caller stores the returned handles in the weapon.
	 *
	 * @param InDefaultWeaponAbilities      Ability infos from FWarriorHeroWeaponData.
	 * @param ApplyLevel                    Level at which abilities are granted.
	 * @param OutGameplayAbilitySpecHandles One handle per granted ability — store in the weapon
	 *                                      and pass to RemoveGrantedHeroWeaponAbilities on unequip.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Ability")
	void GrantHeroWeaponAbilities(const TArray<FHeroGameplayAbilityInfo>& InDefaultWeaponAbilities,
	                              int32 ApplyLevel,
	                              TArray<FGameplayAbilitySpecHandle>& OutGameplayAbilitySpecHandles);

	/**
	 * Revokes all abilities identified by the provided spec handles and empties the array.
	 * Called by UGA_Hero_UnequipAxe using handles retrieved from the weapon being unequipped.
	 * Passing the array by ref allows this method to clear it after revocation, preventing
	 * stale handles from being used again.
	 *
	 * @param InOutSpecHandles  Handles to revoke. Must not be empty. Emptied on return.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Ability")
	void RemoveGrantedHeroWeaponAbilities(UPARAM(ref) TArray<FGameplayAbilitySpecHandle>& InOutSpecHandles);

	/**
	 * Finds all activatable ability specs whose tags match AbilityTagToActivate and attempts
	 * to activate one at random. If multiple abilities match the tag (e.g., a pool of
	 * attack variations), a random one is selected to add unpredictability to AI behavior.
	 * No-op if the randomly selected ability is already active.
	 *
	 * @param AbilityTagToActivate  The tag to search for in the activatable ability list.
	 *                              Must be valid. Uses GetSingleTagContainer for the query.
	 * @return                      True if TryActivateAbility succeeded, false if no matching
	 *                              ability was found or the selected ability was already active.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Ability")
	bool TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate);
};
