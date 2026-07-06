// DataAsset_EnemyStartupData.cpp
// Grants base startup abilities via Super, then iterates EnemyCombatAbilities
// to grant enemy-specific combat abilities directly to the ASC.
// Enemies don't use input tag routing — abilities are activated by AI logic.

#include "DataAssets/StartupData/DataAsset_EnemyStartupData.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbilities/WarriorEnemyGameplayAbility.h"

void UDataAsset_EnemyStartupData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
                                                               int32 ApplyLevel)
{
	// Grant ActivateOnGivenAbilities and ReactiveAbilities defined in the base asset.
	Super::GiveToAbilitySystemComponent(InASC,
	                                    ApplyLevel);

	// Early out: no combat abilities configured for this enemy type.
	// Not an error — some enemies may rely entirely on base startup abilities.
	if (EnemyCombatAbilities.IsEmpty())
	{
		return;
	}

	for (const TSubclassOf<UWarriorEnemyGameplayAbility>& AbilityClass : EnemyCombatAbilities)
	{
		// Null entry in the array is always a designer configuration error —
		// crash fast so it gets caught before a playtest.
		check(AbilityClass);

		// Build the spec without injecting an input tag — enemies activate
		// abilities via AI logic, not player input, so no tag routing is needed.
		FGameplayAbilitySpec AbilitySpec(AbilityClass);
		AbilitySpec.SourceObject = InASC->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;

		InASC->GiveAbility(AbilitySpec);
	}
}
