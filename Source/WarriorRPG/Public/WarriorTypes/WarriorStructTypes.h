// WarriorStructTypes.h
// Centralized shared struct definitions for the WarriorRPG project.
// Structs used across multiple systems (weapons, abilities, UI) live here
// to avoid circular header dependencies between those systems.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "WarriorStructTypes.generated.h"

class UInputMappingContext;
class UWarriorHeroGameplayAbility;
class UWarriorHeroLinkedAnimLayer;

/**
 * Pairs a Gameplay Tag (input identifier) with a Gameplay Ability class.
 * Used in two contexts:
 *   - UDataAsset_HeroStartupData: abilities granted at possession time.
 *   - FWarriorHeroWeaponData: abilities granted when the weapon is equipped.
 *
 * Restricted to UWarriorHeroGameplayAbility (not the base UWarriorGameplayAbility)
 * because this struct is exclusively used in hero contexts. This prevents designers
 * from accidentally assigning enemy or generic abilities in hero DataAssets.
 *
 * At grant time, InputTag is injected into the FGameplayAbilitySpec's dynamic source tags
 * so the ASC can find and activate the ability by tag when the corresponding input fires.
 */
USTRUCT(BlueprintType)
struct FHeroGameplayAbilityInfo
{
	GENERATED_BODY()

	/**
	 * The input tag that identifies this ability's activation trigger.
	 * Must match an entry in DataAsset_InputConfig::GameplayAbilityInputActions.
	 * Filtered to WarriorRPGTags.Input hierarchy for designer clarity.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		meta = (Categories = "WarriorRPGTags.Input"))
	FGameplayTag InputTag;

	/**
	 * The Hero Gameplay Ability class to grant and associate with the input tag.
	 * Restricted to UWarriorHeroGameplayAbility subclasses — ensures all hero abilities
	 * have access to hero character/controller caches and hero combat component helpers.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly)
	TSubclassOf<UWarriorHeroGameplayAbility> GameplayAbility;

	/**
	 * Returns true if both the input tag and the ability class are set and valid.
	 * Used as a guard before building an FGameplayAbilitySpec in GiveToAbilitySystemComponent.
	 */
	bool IsValid() const;
};

/**
 * Data bundle attached to every hero weapon.
 * Defines everything the equip ability needs to fully set up this weapon:
 * the animation layer to activate, the input context to push, and the
 * abilities to grant to the ASC while the weapon is held.
 *
 * All three are reverted when the weapon is unequipped.
 */
USTRUCT(BlueprintType)
struct FWarriorHeroWeaponData
{
	GENERATED_BODY()

	/**
	 * The Linked Anim Layer class to activate when this weapon is equipped.
	 * Assign the weapon-specific Animation Blueprint (e.g., ABP_Hero_Axe) here.
	 * Deactivated via UnlinkAnimClassLayers() when the weapon is unequipped.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly)
	TSubclassOf<UWarriorHeroLinkedAnimLayer> WeaponAnimLayer;

	/**
	 * Input Mapping Context to push when this weapon is equipped.
	 * Contains weapon-specific bindings (light attack, heavy attack, block)
	 * that are only active while this weapon is in hand.
	 * Removed from the Enhanced Input subsystem when the weapon is unequipped.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> WeaponIMC;

	/**
	 * Gameplay Abilities to grant to the ASC when this weapon is equipped.
	 * Each entry pairs an input tag with an ability class — the same mechanism
	 * used in UDataAsset_HeroStartupData for startup abilities.
	 * These abilities are revoked when the weapon is unequipped.
	 * TitleProperty = "InputTag" shows the tag name in editor array entries.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly,
		meta = (TitleProperty = "InputTag"))
	TArray<FHeroGameplayAbilityInfo> DefaultWeaponGameplayAbilities;

	/** Base damage curve for this weapon evaluated at ability level.
	 *  Used by MakeHeroDamageEffectSpecHandle to inject the correct damage into the GE spec. */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly)
	FScalableFloat WeaponBaseDamage;

	/**
	 * Soft reference to the weapon's icon texture.
	 * Loaded asynchronously when the weapon is equipped — broadcast via
	 * UHeroUIComponent::OnEquippedWeaponChanged so the HUD can display
	 * the correct weapon icon without blocking the game thread.
	 */
	UPROPERTY(EditDefaultsOnly,
		BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SoftWeaponIconTexture;
};
