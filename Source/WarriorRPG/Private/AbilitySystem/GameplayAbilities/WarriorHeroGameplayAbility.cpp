// WarriorHeroGameplayAbility.cpp
// Implements cached, type-safe getters for the hero character, controller,
// and combat component used by hero-specific gameplay abilities.

#include "AbilitySystem/GameplayAbilities/WarriorHeroGameplayAbility.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "Controllers/WarriorHeroController.h"
#include "Utils/WarriorRPGTags.h"

AWarriorHeroCharacter* UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo()
{
    // Cache on first access. The cast is only performed once per ability instance,
    // and subsequent calls hit the cached weak pointer directly.
    // IsValid() returns false for both null and pending-kill objects, so if the
    // character was destroyed, we'll redo the cast (which will fail the check below).
    if (!CachedWarriorHeroCharacter.IsValid())
    {
        CachedWarriorHeroCharacter = Cast<AWarriorHeroCharacter>(CurrentActorInfo->AvatarActor.Get());

        // Hard invariant: a hero ability must always run on a hero character.
        // If this check fires, the ability was granted to the wrong character type —
        // a Blueprint setup error that must be caught immediately in development.
        checkf(CachedWarriorHeroCharacter.IsValid(),
               TEXT("UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo — " "AvatarActor is not a valid AWarriorHeroCharacter. " "Verify this ability is only granted to hero characters."));
    }

    return CachedWarriorHeroCharacter.Get();
}

AWarriorHeroController* UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo()
{
    // Same caching pattern as the character getter — cast once, reuse forever.
    if (!CachedWarriorHeroController.IsValid())
    {
        CachedWarriorHeroController = Cast<AWarriorHeroController>(CurrentActorInfo->PlayerController.Get());

        // Hard invariant: a hero ability must always have a hero controller.
        // If this check fires, the player controller class is not set to
        // AWarriorHeroController in the game mode — a project setup error.
        checkf(CachedWarriorHeroController.IsValid(),
               TEXT("UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo — " "PlayerController is not a valid AWarriorHeroController. " "Verify the game mode assigns AWarriorHeroController as the player controller class."));
    }

    return CachedWarriorHeroController.Get();
}

UHeroCombatComponent* UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo()
{
    // Delegate to the character getter — if the character is valid, its combat component
    // is guaranteed to exist because it's created as a default subobject in the constructor.
    // No separate cache needed: the character cache already protects this path.
    AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();

    if (!ensureMsgf(HeroCharacter,
                    TEXT("UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo — " "HeroCharacter is null. Cannot retrieve combat component.")))
    {
        return nullptr;
    }

    UHeroCombatComponent* HeroCombatComponent = HeroCharacter->GetHeroCombatComponent();

    ensureMsgf(HeroCombatComponent,
               TEXT("UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo — " "HeroCombatComponent is null." ));

    return HeroCombatComponent;
}

FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::MakeHeroDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> InEffectClass,
                                                                                      float InWeaponBaseDamage,
                                                                                      FGameplayTag InCurrentAttackTypeTag,
                                                                                      int32 InUsedComboCount)
{
    check(InEffectClass);

    UWarriorAbilitySystemComponent* WarriorASC = GetWarriorAbilitySystemComponentFromActorInfo();
    check(WarriorASC);

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    check(AvatarActor);

    // Build the effect context with full source attribution.
    // SetAbility links this spec to the ability instance for debugging and cancellation queries.
    // AddInstigator is required for damage attribution — GAS uses it to resolve
    // who caused the damage when the effect is applied on the target.
    FGameplayEffectContextHandle ContextHandle = WarriorASC->MakeEffectContext();
    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(AvatarActor);
    ContextHandle.AddInstigator(AvatarActor,
                                AvatarActor);

    // Create the outgoing spec at this ability's current level.
    // Level affects any curve-based modifiers defined inside the GE asset.
    FGameplayEffectSpecHandle EffectSpecHandle = WarriorASC->MakeOutgoingSpec(InEffectClass,
                                                                              GetAbilityLevel(),
                                                                              ContextHandle);

    // Inject base damage as a SetByCaller magnitude — the GE reads this tag at
    // application time instead of using a hard-coded value in the asset.
    // This keeps damage values data-driven and weapon-specific.
    EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorRPGTags::Shared::SetByCaller::BaseDamage,
                                                   InWeaponBaseDamage);

    // Inject the combo count under the attack type tag.
    // The GE uses this to scale damage per combo step — each hit in the chain
    // can deal progressively more damage without needing a separate GE per step.
    check(InCurrentAttackTypeTag.IsValid());
    EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag,
                                                   static_cast<float>(InUsedComboCount));

    return EffectSpecHandle;
}
