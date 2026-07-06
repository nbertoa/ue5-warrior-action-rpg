// DataAsset_EnemyStartupData.h
// Enemy-specific startup data asset.
// Extends the base startup data with combat abilities granted directly to the ASC
// without input tag routing — enemies activate abilities via AI, not player input.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartupData/DataAsset_StartupDataBase.h"
#include "DataAsset_EnemyStartupData.generated.h"

class UWarriorEnemyGameplayAbility;
class UWarriorAbilitySystemComponent;

/**
 * UDataAsset_EnemyStartupData
 *
 * Startup data asset for enemy characters.
 * Defines the initial ability loadout granted to an enemy's ASC
 * when it is possessed or spawned.
 *
 * Enemy-specific startup properties (patrol behavior tags,
 * difficulty scaling, aggro range) will be added here as needed.
 */
UCLASS()
class WARRIORRPG_API UDataAsset_EnemyStartupData : public UDataAsset_StartupDataBase
{
	GENERATED_BODY()

public:
	/**
	 * Grants base startup abilities via Super, then grants all
	 * EnemyCombatAbilities to the provided ASC.
	 * Called from AWarriorEnemyCharacter::OnStartupDataLoaded after async load completes.
	 *
	 * @param InASC         The ability system component to grant abilities to. Must not be null.
	 * @param ApplyLevel    The level at which all abilities are granted. Defaults to 1.
	 */
	virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
	                                          int32 ApplyLevel = 1) override;

private:
	/**
	 * Combat abilities specific to this enemy type.
	 * Granted directly to the ASC without input tag routing —
	 * enemies don't use player input to activate abilities.
	 */
	UPROPERTY(EditDefaultsOnly,
		Category = "StartupData")
	TArray<TSubclassOf<UWarriorEnemyGameplayAbility>> EnemyCombatAbilities;
};
