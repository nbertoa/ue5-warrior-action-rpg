// WarriorAbilitySystemComponent.cpp
// Input-driven ability activation and weapon ability lifecycle management.

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbilities/WarriorHeroGameplayAbility.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "Utils/WarriorRPGLogCategories.h"
#include "Utils/WarriorRPGTags.h"

void UWarriorAbilitySystemComponent::OnGameplayAbilityInputPressed(const FGameplayTag& InInputTag)
{
    check(InInputTag.IsValid());

    ABILITYLIST_SCOPE_LOCK();

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        // HasTagExact: exact match only — prevents hierarchy matches from triggering
        // unintended abilities if the tag hierarchy is expanded in the future.
        if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InInputTag))
        {
            continue;
        }

        if (InInputTag.MatchesTag(WarriorRPGTags::Input::Toggleable::Tag) && AbilitySpec.IsActive())
        {
            CancelAbilityHandle(AbilitySpec.Handle);
        }
        else
        {
            TryActivateAbility(AbilitySpec.Handle);
        }

        // One input tag → one spec. Stop after the first match.
        break;
    }
}

void UWarriorAbilitySystemComponent::OnGameplayAbilityInputReleased(const FGameplayTag& InInputTag)
{
    check(InInputTag.IsValid());

    // Only held-input abilities react to button release.
    // MatchesTag checks the full hierarchy — any tag under Input::MustBeHeld
    // automatically qualifies without modifying this function.
    if (!InInputTag.MatchesTag(WarriorRPGTags::Input::MustBeHeld::Tag))
    {
        return;
    }

    ABILITYLIST_SCOPE_LOCK();

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        const bool bHasTagExact = AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InInputTag);

        // Cancel only the ability that owns this exact input tag and is currently active.
        // HasTagExact prevents hierarchy spillover — releasing Block must not cancel
        // an unrelated held ability that happens to share a parent tag.
        if (bHasTagExact && AbilitySpec.IsActive())
        {
            CancelAbilityHandle(AbilitySpec.Handle);
        }
    }
}

void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(const TArray<FHeroGameplayAbilityInfo>& InDefaultWeaponAbilities,
                                                              int32 ApplyLevel,
                                                              TArray<FGameplayAbilitySpecHandle>& OutGameplayAbilitySpecHandles)
{
    if (!ensureMsgf(!InDefaultWeaponAbilities.IsEmpty(),
                    TEXT("UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities — " "InDefaultWeaponAbilities is empty. " "Add ability entries to the weapon's DefaultWeaponGameplayAbilities array.")))
    {
        return;
    }

    for (const FHeroGameplayAbilityInfo& AbilityInfo : InDefaultWeaponAbilities)
    {
        check(AbilityInfo.IsValid());

        FGameplayAbilitySpec AbilitySpec(AbilityInfo.GameplayAbility);
        AbilitySpec.SourceObject = GetAvatarActor();
        AbilitySpec.Level = ApplyLevel;

        // DynamicSpecSourceTags keeps input routing unified with startup abilities.
        // OnGameplayAbilityInputPressed searches this container exclusively.
        AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityInfo.InputTag);

        OutGameplayAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
    }
}

void UWarriorAbilitySystemComponent::RemoveGrantedHeroWeaponAbilities(TArray<FGameplayAbilitySpecHandle>& InOutSpecHandles)
{
    if (!ensureMsgf(!InOutSpecHandles.IsEmpty(),
                    TEXT("UWarriorAbilitySystemComponent::RemoveGrantedHeroWeaponAbilities — " "InOutSpecHandles is empty. " "Ensure GrantHeroWeaponAbilities was called when the weapon was equipped.")))
    {
        return;
    }

    for (const FGameplayAbilitySpecHandle& SpecHandle : InOutSpecHandles)
    {
        check(SpecHandle.IsValid());
        ClearAbility(SpecHandle);
    }

    // Empty the array so stale handles cannot be reused accidentally.
    InOutSpecHandles.Empty();
}

bool UWarriorAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate)
{
    check(AbilityTagToActivate.IsValid());

    TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;

    // GetSingleTagContainer wraps the tag in a container for the query API —
    // the ability's OwnedGameplayTags must contain this tag for it to be returned.
    GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTagToActivate.GetSingleTagContainer(),
                                                        FoundAbilitySpecs);

    if (FoundAbilitySpecs.IsEmpty())
    {
        UE_LOG(LogWarriorRPG,
               Warning,
               TEXT("UWarriorAbilitySystemComponent::TryActivateAbilityByTag — " "No activatable ability found for tag [%s] on [%s]. " "Verify the ability has the tag set in its OwnedGameplayTags and is granted to this ASC."),
               *AbilityTagToActivate.ToString(),
               *GetNameSafe(GetAvatarActor()));

        return false;
    }

    // When multiple abilities match the tag (e.g., a pool of attack variations),
    // selecting one at random adds unpredictability to AI behavior without
    // requiring separate ability classes per variation.
    const int32 RandomAbilityIndex = FMath::RandRange(0,
                                                      FoundAbilitySpecs.Num() - 1);
    FGameplayAbilitySpec* SpecToActivate = FoundAbilitySpecs[RandomAbilityIndex];
    check(SpecToActivate);

    // Skip if already active — avoids double-activation of the same spec,
    // which can cause undefined behavior in the ability's state machine.
    if (SpecToActivate->IsActive())
    {
        return false;
    }

    return TryActivateAbility(SpecToActivate->Handle);
}
