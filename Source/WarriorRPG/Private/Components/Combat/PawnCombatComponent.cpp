// PawnCombatComponent.cpp
// Implements the weapon registry and equipped weapon lookup with strict traceability.

#include "Components/Combat/PawnCombatComponent.h"

#include "Components/BoxComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "Utils/WarriorRPGLogCategories.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTag,
                                                 AWarriorWeaponBase* InWeapon,
                                                 bool bEquippedWeapon)
{
    const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("UnknownOwner");

    // Defensive: prevent duplicate registrations — registering the same tag twice
    // would silently overwrite the existing entry and leak the old weapon's delegates.
    if (!ensureMsgf(!CarriedWeaponByTag.Contains(InWeaponTag),
                    TEXT("[%s] UPawnCombatComponent::RegisterSpawnedWeapon — a weapon with tag " "[%s] is already registered. Ignoring duplicate registration."),
                    *OwnerName,
                    *InWeaponTag.ToString()))
    {
        return;
    }

    // Defensive: a null weapon pointer here means the spawn failed upstream.
    // Storing null would cause GetCarriedWeaponByTag to silently return an invalid pointer.
    if (!ensureMsgf(IsValid(InWeapon),
                    TEXT("[%s] UPawnCombatComponent::RegisterSpawnedWeapon — InWeapon is null " "for tag [%s]. Verify the weapon was spawned successfully before registration."),
                    *OwnerName,
                    *InWeaponTag.ToString()))
    {
        return;
    }

    CarriedWeaponByTag.Add(InWeaponTag,
                           InWeapon);

    // Bind the weapon's hit delegates to this component so the weapon itself
    // stays agnostic of combat logic — it only reports collisions; this component
    // decides what to do with them.
    InWeapon->OnWeaponHitTarget.BindUObject(this,
                                            &ThisClass::OnHitTargetActor);
    InWeapon->OnWeaponPulledFromTarget.BindUObject(this,
                                                   &ThisClass::OnWeaponPulledFromTargetActor);

    if (bEquippedWeapon)
    {
        CurrentEquippedWeaponTag = InWeaponTag;
    }

    UE_LOG(LogWarriorRPG,
           Log,
           TEXT("[%s] Weapon [%s] registered under tag [%s]%s."),
           *OwnerName,
           *InWeapon->GetName(),
           *InWeaponTag.ToString(),
           bEquippedWeapon ? TEXT(" and equipped") : TEXT(""));
}

AWarriorWeaponBase* UPawnCombatComponent::GetCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
    // Find returns a pointer to the value in the map, or nullptr if the key is absent.
    // We use Find instead of operator[] because operator[] inserts a default entry
    // for missing keys, which would silently corrupt the registry in a const accessor.
    if (const TObjectPtr<AWarriorWeaponBase>* FoundWeapon = CarriedWeaponByTag.Find(InWeaponTag))
    {
        return *FoundWeapon;
    }

    const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("UnknownOwner");

    UE_LOG(LogWarriorRPG,
           Warning,
           TEXT("[%s] UPawnCombatComponent::GetCarriedWeaponByTag — no weapon registered " "under tag [%s]."),
           *OwnerName,
           *InWeaponTag.ToString());

    return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCurrentEquippedWeapon() const
{
    // An invalid tag means no weapon has been equipped yet — return early
    // rather than forwarding an invalid tag to GetCarriedWeaponByTag,
    // which would log a spurious warning.
    if (!CurrentEquippedWeaponTag.IsValid())
    {
        return nullptr;
    }

    return GetCarriedWeaponByTag(CurrentEquippedWeaponTag);
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable,
                                                 EToggleDamageType ToggleDamageType)
{
    if (ToggleDamageType == EToggleDamageType::CurrentEquippedWeapon)
    {
        ToggleCurrentEquippedWeaponCollision(bShouldEnable);
    }
    else
    {
        // LeftHand and RightHand are body collision boxes owned by the character,
        // not weapon actors — delegate to the subclass that knows about them.
        ToggleBodyCollisionBoxCollision(bShouldEnable,
                                        ToggleDamageType);
    }
}

void UPawnCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    // Base implementation is intentionally empty — subclasses provide
    // faction-specific hit reactions (hero vs enemy combat logic).
}

void UPawnCombatComponent::OnWeaponPulledFromTargetActor(AActor* InteractedActor)
{
    // Base implementation is intentionally empty — override in subclasses
    // if per-hit state needs to be cleaned up when the weapon leaves a target.
}

void UPawnCombatComponent::ToggleCurrentEquippedWeaponCollision(bool bShouldEnable)
{
    AWarriorWeaponBase* WeaponToToggle = GetCurrentEquippedWeapon();
    check(WeaponToToggle);

    UBoxComponent* BoxCollision = WeaponToToggle->GetWeaponCollisionBox();
    check(BoxCollision);

    BoxCollision->SetCollisionEnabled(bShouldEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

    // Clear the dedup list when collision is disabled so the next swing
    // starts fresh and can hit the same targets again.
    if (!bShouldEnable)
    {
        OverlappedActors.Empty();
    }
}

void UPawnCombatComponent::ToggleBodyCollisionBoxCollision(bool bShouldEnable,
                                                           EToggleDamageType ToggleDamageType)
{
    // Base implementation is intentionally empty — only subclasses that own
    // body collision boxes (e.g., UEnemyCombatComponent) need to implement this.
}
