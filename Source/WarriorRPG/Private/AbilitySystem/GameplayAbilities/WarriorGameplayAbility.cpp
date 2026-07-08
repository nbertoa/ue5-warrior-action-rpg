// WarriorGameplayAbility.cpp
// Implements the activation policy logic for WarriorRPG abilities.

#include "AbilitySystem/GameplayAbilities/WarriorGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Utils/WarriorRPGLogCategories.h"

void UWarriorGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo,
                         Spec);

    // ActorInfo is provided by the engine and should never be null at this stage.
    // Using ensure instead of check because if this somehow fails, we'd rather log
    // the callstack and skip activation than crash the entire game — losing one
    // ability's auto-activation is recoverable; a crash is not.
    if (!ensureMsgf(ActorInfo,
                    TEXT("%s (UWarriorGameplayAbility): OnGiveAbility — ActorInfo is null. " "This should never happen as the engine provides it."),
                    *GetName()))
    {
        return;
    }

    // Only proceed with auto-activation if this ability uses the OnGiven policy.
    // OnTriggered abilities do nothing here and wait for external activation.
    if (AbilityActivationPolicy != EWarriorAbilityActivationPolicy::OnGiven)
    {
        return;
    }

    // Defensive: verify the ASC exists before attempting to activate through it.
    // ActorInfo->AbilitySystemComponent is a weak pointer, so we check .IsValid() via the ? operator.
    if (!ensureMsgf(ActorInfo->AbilitySystemComponent.IsValid(),
                    TEXT("%s (UWarriorGameplayAbility): OnGiveAbility — AbilitySystemComponent is invalid. " "Cannot auto-activate ability."),
                    *GetName()))
    {
        return;
    }

    // Avoid double-activation: if the spec is already active, another code path
    // (like an explicit TryActivateAbility call) has already triggered it.
    // Activating it again would either fail silently or cause unexpected behavior.
    if (!Spec.IsActive())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
    }
}

void UWarriorGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         bool bReplicateEndAbility,
                                         bool bWasCancelled)
{
    Super::EndAbility(Handle,
                      ActorInfo,
                      ActivationInfo,
                      bReplicateEndAbility,
                      bWasCancelled);

    // Same defensive rationale as OnGiveAbility — ensure over check for graceful failure.
    if (!ensureMsgf(ActorInfo,
                    TEXT("%s (UWarriorGameplayAbility): EndAbility — ActorInfo is null. " "This should never happen as the engine provides it."),
                    *GetName()))
    {
        return;
    }

    // Only abilities with the OnGiven policy need to be fully cleared on end.
    // OnTriggered abilities should remain on the ASC so they can be activated again later.
    if (AbilityActivationPolicy != EWarriorAbilityActivationPolicy::OnGiven)
    {
        return;
    }

    if (!ensureMsgf(ActorInfo->AbilitySystemComponent.IsValid(),
                    TEXT("%s (UWarriorGameplayAbility): EndAbility — AbilitySystemComponent is invalid. " "Cannot clear ability."),
                    *GetName()))
    {
        return;
    }

    // ClearAbility fully removes the ability spec from the ASC.
    // This is intentional for OnGiven abilities: they represent "one-shot lifetime"
    // where activation and removal are tied to the grant-end cycle. Without this,
    // the ability would remain as a dormant spec that can never re-activate
    // (because OnGiveAbility only runs on initial grant, not on re-activation).
    ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
}

UPawnCombatComponent* UWarriorGameplayAbility::GetPawnCombatComponentFromActorInfo() const
{
    // The AvatarActor can legitimately be null if the ability outlives 
    // the pawn (e.g., a delayed effect resolving after the player died).
    // We MUST check it before dereferencing to prevent a hard crash.
    AActor* AvatarActor = CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;

    if (!ensureMsgf(AvatarActor,
                    TEXT("%s (UWarriorGameplayAbility): GetPawnCombatComponentFromActorInfo failed. " "AvatarActor is null. This occurs if the ability is still active after the pawn is destroyed."),
                    *GetName()))
    {
        return nullptr;
    }

    // The combat component is a softer expectation: we expect it to be there
    // because this helper is designed for combat-oriented abilities, but using
    // ensureMsgf instead of check lets non-combat callers handle the null case
    // gracefully if the design evolves to include non-combat avatars later.
    UPawnCombatComponent* PawnCombatComponent = AvatarActor->FindComponentByClass<UPawnCombatComponent>();

    ensureMsgf(PawnCombatComponent,
               TEXT("%s (UWarriorGameplayAbility): Avatar [%s] has no UPawnCombatComponent. " "This ability was designed to run on combat-capable pawns."),
               *GetName(),
               *AvatarActor->GetName());

    return PawnCombatComponent;
}

UWarriorAbilitySystemComponent* UWarriorGameplayAbility::GetWarriorAbilitySystemComponentFromActorInfo() const
{
    // CurrentActorInfo is set by the engine while the ability is active.
    // Hard invariant: if this getter is called, the ability is executing and
    // the engine guarantees CurrentActorInfo is valid.
    check(CurrentActorInfo);

    // The ASC inside ActorInfo is a weak pointer — we verify it's still valid
    // before dereferencing. If it's not, something went very wrong (the character
    // was destroyed mid-ability?) and crashing is the right response.
    check(CurrentActorInfo->AbilitySystemComponent.IsValid());

    // Cast to our project-specific ASC type. All characters in WarriorRPG use
    // UWarriorAbilitySystemComponent, so this cast must succeed — a failure would
    // indicate a character was set up with the wrong ASC subclass.
    UWarriorAbilitySystemComponent* ASC = Cast<UWarriorAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
    check(ASC);

    return ASC;
}

FActiveGameplayEffectHandle UWarriorGameplayAbility::NativeApplyEffectSpecHandleToTarget(AActor* InTargetActor,
                                                                                         const FGameplayEffectSpecHandle& InSpecHandle)
{
    // An invalid spec handle means the effect was never properly built —
    // catching this here avoids a harder-to-diagnose failure inside ApplyGameplayEffectSpecToTarget.
    check(InSpecHandle.IsValid());

    // Retrieve the target's ASC — required to receive the effect.
    // check instead of ensureMsgf: calling this function on an actor without an ASC
    // is a programming error in the ability, not a designer configuration mistake.
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InTargetActor);
    check(TargetASC);

    // Apply the spec from our ASC to the target's ASC.
    // Using ApplyGameplayEffectSpecToTarget (source → target) rather than
    // ApplyGameplayEffectSpecToSelf because damage effects originate from the
    // instigator and need correct source attribution for damage calculation and UI.
    UWarriorAbilitySystemComponent* WarriorASC = GetWarriorAbilitySystemComponentFromActorInfo();
    check(WarriorASC);

    return WarriorASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data,
                                                       TargetASC);
}

FActiveGameplayEffectHandle UWarriorGameplayAbility::BP_ApplyEffectSpecHandleToTarget(AActor* InTargetActor,
                                                                                      const FGameplayEffectSpecHandle& InSpecHandle,
                                                                                      EWarriorSuccessType& OutSuccessType)
{
    check(InSpecHandle.IsValid());

    // Delegate to the Native variant — all real logic lives there.
    // This wrapper exists solely to provide the ExpandEnumAsExecs output pin
    // that Blueprint needs. Keeping the implementation in the Native function
    // avoids duplicating defensive code and keeps C++ callers fast.
    FActiveGameplayEffectHandle ActiveGEHandle = NativeApplyEffectSpecHandleToTarget(InTargetActor,
                                                                                     InSpecHandle);

    // WasSuccessfullyApplied returns false if the target's ASC rejected the effect
    // (e.g., immune tag, effect inhibited, or target died between spec creation and application).
    // Converting to enum here rather than returning a bool keeps the BP node
    // consistent with the rest of the library's ExpandEnumAsExecs pattern.
    OutSuccessType = ActiveGEHandle.WasSuccessfullyApplied() ? EWarriorSuccessType::Succeeded : EWarriorSuccessType::Failed;

    return ActiveGEHandle;
}
