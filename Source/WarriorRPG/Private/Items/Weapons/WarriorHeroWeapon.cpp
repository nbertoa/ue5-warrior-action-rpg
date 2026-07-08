// WarriorHeroWeapon.cpp
// Manages the lifecycle of granted ability spec handles for the hero weapon.
// The equip ability assigns handles after granting; the unequip ability retrieves
// them for revocation. EndPlay provides a last-resort cleanup path.

#include "Items/Weapons/WarriorHeroWeapon.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorBaseCharacter.h"
#include "Utils/WarriorRPGLogCategories.h"

void AWarriorHeroWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InSpecHandles)
{
    // Empty handles indicate GrantHeroWeaponAbilities produced no results —
    // always an authoring error (weapon has no abilities configured).
    check(!InSpecHandles.IsEmpty());

    GrantedAbilitySpecHandles = InSpecHandles;
}

const TArray<FGameplayAbilitySpecHandle>& AWarriorHeroWeapon::GetGrantedAbilitySpecHandles() const
{
    return GrantedAbilitySpecHandles;
}

void AWarriorHeroWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Failsafe: if handles are still active here, the normal unequip path was skipped
    // (character death during equip animation, level unload, editor PIE stop, etc.).
    // Force-revoke to prevent orphaned specs — specs that remain in the ASC indefinitely
    // with no way to be activated or cleaned up through the normal gameplay flow.
    if (!GrantedAbilitySpecHandles.IsEmpty())
    {
        AWarriorBaseCharacter* OwningCharacter = Cast<AWarriorBaseCharacter>(GetOwner());
        if (!OwningCharacter)
        {
            // Weapon may have been detached from its owner before EndPlay — not a fatal error.
            UE_LOG(LogWarriorRPG,
                   Warning,
                   TEXT("%s: EndPlay — GrantedAbilitySpecHandles is not empty but owner is null " "or not a AWarriorBaseCharacter. Ability specs may be orphaned."),
                   *GetName());

            // Still call Super so the actor lifecycle chain is not broken.
            Super::EndPlay(EndPlayReason);
            return;
        }

        UWarriorAbilitySystemComponent* ASC = OwningCharacter->GetWarriorAbilitySystemComponent();
        if (ASC)
        {
            ASC->RemoveGrantedHeroWeaponAbilities(GrantedAbilitySpecHandles);
        }
        else
        {
            UE_LOG(LogWarriorRPG,
                   Warning,
                   TEXT("%s: EndPlay — Could not retrieve ASC from owning character [%s]. " "Ability specs may be orphaned."),
                   *GetName(),
                   *OwningCharacter->GetName());
        }
    }

    // Super called last — weapon must still be in valid state during cleanup above.
    Super::EndPlay(EndPlayReason);
}
