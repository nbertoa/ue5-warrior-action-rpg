// DataAsset_HeroStartupData.h
// Hero-specific startup data asset.
// Extends the base startup data with ability-input bindings: each entry pairs
// a Gameplay Tag (the input identifier) with a Gameplay Ability class.
// The tag is injected into the FGameplayAbilitySpec at grant time so the ASC
// can later find and activate the ability by tag when the player presses the button.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartupData/DataAsset_StartupDataBase.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "DataAsset_HeroStartupData.generated.h"

/**
 * Hero-specific startup data asset.
 * Inherits ActivateOnGivenAbilities and ReactiveAbilities from the base class,
 * and adds HeroStartupGameplayAbilityInfos for input-bound abilities granted at possession.
 *
 * Note: weapon-granted abilities (attacks, blocks) are defined in FWarriorHeroWeaponData
 * and granted by the equip ability — not here. This asset only covers abilities that
 * the hero always has, regardless of what weapon is equipped.
 *
 * Exists as a separate type so hero characters can only be assigned hero-appropriate
 * startup data in the editor — a type-safety guardrail for designers.
 */
UCLASS()
class WARRIORRPG_API UDataAsset_HeroStartupData : public UDataAsset_StartupDataBase
{
	GENERATED_BODY()

public:
	/**
	 * Grants all startup abilities to the given ASC.
	 * Calls Super to handle ActivateOnGivenAbilities and ReactiveAbilities,
	 * then iterates HeroStartupGameplayAbilityInfos to grant input-bound abilities
	 * with their tags injected into the spec's dynamic source tags.
	 *
	 * @param InASC         The ability system component to grant abilities to. Must not be null.
	 * @param ApplyLevel    The level at which all abilities are granted.
	 */
	virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
	                                          int32 ApplyLevel = 1) override;

private:
	/**
	 * Input-bound abilities granted to the hero at possession time.
	 * These are always present regardless of equipped weapon — e.g., the equip/unequip
	 * abilities themselves, dodge, or any other weapon-agnostic action.
	 * TitleProperty = "InputTag" shows the tag name in editor array entries.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "StartupData",
		meta = (TitleProperty = "InputTag"))
	TArray<FHeroGameplayAbilityInfo> HeroStartupGameplayAbilityInfos;
};
