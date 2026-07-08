// DataAsset_StartupDataBase.cpp
// Implements the ability-granting pipeline for character startup.

#include "DataAssets/StartupData/DataAsset_StartupDataBase.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbilities/WarriorGameplayAbility.h"

void UDataAsset_StartupDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
                                                              int32 ApplyLevel)
{
    // The ASC is a required parameter — without it we can't grant anything.
    // Using ensureMsgf instead of check because a null ASC is a caller error
    // that should be logged and escaped from, not cause a hard crash.
    if (!ensureMsgf(InASC,
                    TEXT("%s (UDataAsset_StartupDataBase): GiveToAbilitySystemComponent — InWarriorASC is null. " "Verify the owning character created its ASC in the constructor."),
                    *GetName()))
    {
        return;
    }

    // Grant both ability lists. The distinction between them is organizational —
    // designers use ActivateOnGivenAbilities for passives and ReactiveAbilities
    // for input-driven or event-driven ones, but both use the same GrantAbilities path.
    GrantAbilities(ActivateOnGivenAbilities,
                   InASC,
                   ApplyLevel);
    GrantAbilities(ReactiveAbilities,
                   InASC,
                   ApplyLevel);

    for (const TSubclassOf<UGameplayEffect>& EffectClass : StartUpGameplayEffects)
    {
        check(EffectClass);

        // GetDefaultObject is safe here — EffectClass is guaranteed non-null above,
        // and CDOs are always valid for registered classes.
        UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

        InASC->ApplyGameplayEffectToSelf(EffectCDO,
                                         ApplyLevel,
                                         InASC->MakeEffectContext());
    }
}

void UDataAsset_StartupDataBase::GrantAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilities,
                                                UWarriorAbilitySystemComponent* InWarriorASC,
                                                int32 ApplyLevel)
{
    for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilities)
    {
        // Defensive: a null entry in the array is a designer configuration error.
        // Using ensure instead of check so the loop can skip the bad entry and
        // continue granting the valid ones — losing one ability is recoverable,
        // a crash in the middle of character initialization is not.
        if (!ensureMsgf(Ability,
                        TEXT("%s (UDataAsset_StartupDataBase): Found a null ability entry in GrantAbilities. " "Check the StartupData asset for empty slots in the ability arrays."),
                        *GetName()))
        {
            continue;
        }

        // Build the ability spec with the level and source actor.
        // SourceObject is used by GAS to track which actor originated this ability grant —
        // useful for damage attribution, UI icons, and debugging.
        FGameplayAbilitySpec AbilitySpec(Ability);
        AbilitySpec.SourceObject = InWarriorASC->GetAvatarActor();
        AbilitySpec.Level = ApplyLevel;

        // GiveAbility registers the ability in the ASC's ability list.
        // If the ability's activation policy is OnGiven, it will auto-activate
        // immediately from inside UWarriorGameplayAbility::OnGiveAbility.
        InWarriorASC->GiveAbility(AbilitySpec);
    }
}
