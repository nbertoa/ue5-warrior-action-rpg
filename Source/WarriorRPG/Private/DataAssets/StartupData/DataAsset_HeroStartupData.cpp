// DataAsset_HeroStartupData.cpp
// Grants hero-specific input-bound abilities to the ASC at possession time.
// The key operation is injecting the InputTag into FGameplayAbilitySpec's dynamic
// source tags — this is what allows the ASC to find the ability by tag later.

#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "AbilitySystem/GameplayAbilities/WarriorHeroGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"

void UDataAsset_HeroStartupData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
                                                              int32 ApplyLevel)
{
    // Grant ActivateOnGivenAbilities and ReactiveAbilities defined in the base asset.
    Super::GiveToAbilitySystemComponent(InASC,
                                        ApplyLevel);

    // Explicit guard even though the base class validates InASC — defensive redundancy
    // ensures this function is self-contained and safe if called directly in isolation.
    check(InASC);

    for (const FHeroGameplayAbilityInfo& GAInfo : HeroStartupGameplayAbilityInfos)
    {
        // Invalid entries are always an authoring mistake — crash fast in development
        // so the designer fixes the DataAsset before it reaches a playtest.
        check(GAInfo.IsValid());

        FGameplayAbilitySpec AbilitySpec(GAInfo.GameplayAbility);
        AbilitySpec.SourceObject = InASC->GetAvatarActor();
        AbilitySpec.Level = ApplyLevel;

        // Inject the input tag into the spec's dynamic source tags.
        // This is what allows OnGameplayAbilityInputPressed to find this spec
        // by tag when the player presses the corresponding button.
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(GAInfo.InputTag);

        InASC->GiveAbility(AbilitySpec);
    }
}
