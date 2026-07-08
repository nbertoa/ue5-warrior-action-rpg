// EnemyCombatComponent.cpp
// Enemy-specific combat logic — hit detection, block evaluation, and body collision toggling.

#include "Components/Combat/EnemyCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/WarriorEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Utils/WarriorRPGDebugHelper.h"
#include "Utils/WarriorRPGTags.h"
#include "WarriorFunctionLibrary.h"

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    check(HitActor);

    APawn* OwningPawn = GetOwningPawn();
    check(OwningPawn);

    // Deduplicate hits within a single attack swing — OverlappedActors is cleared
    // by the base class when weapon collision is disabled at the end of the swing.
    if (OverlappedActors.Contains(HitActor))
    {
        return;
    }
    OverlappedActors.AddUnique(HitActor);

    // Unblockable attacks bypass the block check entirely regardless of the defender's
    // facing or block state. The tag lives on the attacking pawn (not the ability),
    // so any ability active while this tag is set inherits the unblockable behavior.
    const bool bIsMyAttackUnblockable = UWarriorFunctionLibrary::NativeDoesActorHaveTag(OwningPawn,
                                                                                        WarriorRPGTags::Enemy::Status::Unblockable);

    bool bIsValidBlock = false;

    if (!bIsMyAttackUnblockable)
    {
        // Check if the defender is in a blocking state before running the geometry test —
        // avoids a dot product calculation on every hit when the player is not blocking at all.
        const bool bIsTargetBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitActor,
                                                                                       WarriorRPGTags::Player::Status::Blocking);

        if (bIsTargetBlocking)
        {
            // Tag check alone is insufficient — the player could be blocking while
            // facing the wrong direction. IsValidBlock confirms the defender's forward
            // vector is geometrically oriented toward the attacker (dot threshold: -0.1f).
            bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(OwningPawn,
                                                                  HitActor);
        }
    }

    FGameplayEventData EventData;
    EventData.Instigator = OwningPawn;
    EventData.Target = HitActor;

    if (bIsValidBlock)
    {
        // Route the event to the DEFENDER — the block ability on the hero listens
        // for this tag via WaitGameplayEvent to trigger its block reaction logic
        // and suppress the incoming damage application.
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor,
                                                                 WarriorRPGTags::Player::Event::SuccessfulBlock,
                                                                 EventData);
    }
    else
    {
        // No valid block (or unblockable attack) — route the event to the ATTACKER
        // so the active attack ability can proceed with damage application
        // via WaitGameplayEvent(Shared::Event::MeleeHit).
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwningPawn,
                                                                 WarriorRPGTags::Shared::Event::MeleeHit,
                                                                 EventData);
    }
}

void UEnemyCombatComponent::ToggleBodyCollisionBoxCollision(bool bShouldEnable,
                                                            EToggleDamageType ToggleDamageType)
{
    AWarriorEnemyCharacter* OwningEnemyCharacter = GetOwningPawn<AWarriorEnemyCharacter>();
    check(OwningEnemyCharacter);

    UBoxComponent* LeftHandCollisionBox = OwningEnemyCharacter->GetLeftHandCollisionBox();
    check(LeftHandCollisionBox);

    UBoxComponent* RightHandCollisionBox = OwningEnemyCharacter->GetRightHandCollisionBox();
    check(RightHandCollisionBox);

    const ECollisionEnabled::Type EnabledType = ECollisionEnabled::QueryOnly;
    const ECollisionEnabled::Type DisabledType = ECollisionEnabled::NoCollision;

    switch (ToggleDamageType)
    {
        case EToggleDamageType::LeftHand:
            LeftHandCollisionBox->SetCollisionEnabled(bShouldEnable ? EnabledType : DisabledType);
            break;

        case EToggleDamageType::RightHand:
            RightHandCollisionBox->SetCollisionEnabled(bShouldEnable ? EnabledType : DisabledType);
            break;

        default:
            break;
    }

    // Clear the dedup list when collision is disabled so the next swing
    // starts fresh and can hit the same targets again.
    if (!bShouldEnable)
    {
        OverlappedActors.Empty();
    }
}
