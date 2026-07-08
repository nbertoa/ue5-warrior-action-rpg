// HeroCombatComponent.cpp
// Implementation for UHeroCombatComponent.
// Provides typed weapon access for the hero — delegates lookup to the base class
// and casts the result to AWarriorHeroWeapon for access to hero-specific data.

#include "Components/Combat/HeroCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Items/Weapons/WarriorHeroWeapon.h"
#include "Utils/WarriorRPGTags.h"

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
    // Tag validity is a programming invariant — the caller is responsible for passing
    // a valid tag. An invalid tag can never match any registered weapon.
    check(InWeaponTag.IsValid());

    // Delegate the lookup to the base class, which searches the weapon registry TMap.
    // CastChecked instead of Cast: if the weapon registered under this tag is not an
    // AWarriorHeroWeapon, that means the wrong weapon class was spawned and registered —
    // a Blueprint setup error that should crash in development for immediate visibility.
    return CastChecked<AWarriorHeroWeapon>(GetCarriedWeaponByTag(InWeaponTag),
                                           ECastCheckedType::NullAllowed);
}

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCurrentEquippedWeapon() const
{
    // Cast instead of CastChecked here: GetCurrentEquippedWeapon() can legitimately
    // return null if no weapon is equipped yet, and the caller is expected to handle that.
    // A failed cast to AWarriorHeroWeapon would also return null, preserving that contract.
    return Cast<AWarriorHeroWeapon>(GetCurrentEquippedWeapon());
}

float UHeroCombatComponent::GetHeroCurrentEquippedWeaponDamageAtLevel(float InLevel) const
{
    AWarriorHeroWeapon* Weapon = GetHeroCurrentEquippedWeapon();

    // Hard invariant: calling this function without an equipped weapon is a programming
    // error — the ability system should never request damage values when no weapon is active.
    check(Weapon);

    // WeaponBaseDamage is a FScalableFloat backed by a curve table row.
    // GetValueAtLevel evaluates the curve at InLevel, allowing damage to scale
    // with ability level without modifying the weapon asset.
    return Weapon->HeroWeaponData.WeaponBaseDamage.GetValueAtLevel(InLevel);
}

void UHeroCombatComponent::OnHitTargetActor(AActor* HitActor)
{
    // Dedup check: OverlappedActors tracks which actors were already hit in this swing.
    // Without this, a single slow swing could hit the same enemy multiple times
    // as the box component fires repeated overlap events on the same target.
    if (OverlappedActors.Contains(HitActor))
    {
        return;
    }

    OverlappedActors.Add(HitActor);

    // Build the event payload with full source attribution.
    // The active attack ability listens for this event via WaitGameplayEvent(MeleeHit)
    // and uses Instigator/Target to apply the damage GE to the correct actor.
    FGameplayEventData Data;
    Data.Instigator = GetOwningPawn();
    Data.Target = HitActor;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),
                                                             WarriorRPGTags::Shared::Event::MeleeHit,
                                                             Data);

    // HitPause carries no payload — a named empty struct makes the intent explicit
    // and avoids an anonymous temporary per Epic's API design guidelines.
    const FGameplayEventData EmptyEventData;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),
                                                             WarriorRPGTags::Player::Event::HitPause,
                                                             EmptyEventData);
}

void UHeroCombatComponent::OnWeaponPulledFromTargetActor(AActor* InteractedActor)
{
    // HitPause carries no payload — a named empty struct makes the intent explicit
    // and avoids an anonymous temporary per Epic's API design guidelines.
    const FGameplayEventData EmptyEventData;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwningPawn(),
                                                             WarriorRPGTags::Player::Event::HitPause,
                                                             EmptyEventData);
}
