// WarriorEnemyGameplayAbility.cpp
// Implementation for UWarriorEnemyGameplayAbility.
// Enemy-specific ability logic will be added here as the AI combat system evolves.

#include "AbilitySystem/GameplayAbilities/WarriorEnemyGameplayAbility.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Utils/WarriorRPGTags.h"

AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfo()
{
    if (!CachedWarriorEnemyCharacter.IsValid())
    {
        CachedWarriorEnemyCharacter = Cast<AWarriorEnemyCharacter>(CurrentActorInfo->AvatarActor);
        check(CachedWarriorEnemyCharacter.IsValid());
    }

    return CachedWarriorEnemyCharacter.Get();
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponentFromActorInfo()
{
    AWarriorEnemyCharacter* EnemyCharacter = GetEnemyCharacterFromActorInfo();
    check(EnemyCharacter);

    UEnemyCombatComponent* CombatComponent = EnemyCharacter->GetEnemyCombatComponent();
    check(CombatComponent);

    return CombatComponent;
}

FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass,
                                                                                        const FScalableFloat& InDamageScalableFloat)
{
    check(EffectClass);

    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    check(AvatarActor);

    UWarriorAbilitySystemComponent* WarriorASC = GetWarriorAbilitySystemComponentFromActorInfo();
    check(WarriorASC);

    FGameplayEffectContextHandle ContextHandle = WarriorASC->MakeEffectContext();
    ContextHandle.SetAbility(this);
    ContextHandle.AddSourceObject(AvatarActor);
    ContextHandle.AddInstigator(AvatarActor,
                                AvatarActor);

    FGameplayEffectSpecHandle EffectSpecHandle = WarriorASC->MakeOutgoingSpec(EffectClass,
                                                                              GetAbilityLevel(),
                                                                              ContextHandle);

    EffectSpecHandle.Data->SetSetByCallerMagnitude(WarriorRPGTags::Shared::SetByCaller::BaseDamage,
                                                   InDamageScalableFloat.GetValueAtLevel(GetAbilityLevel()));

    return EffectSpecHandle;
}
