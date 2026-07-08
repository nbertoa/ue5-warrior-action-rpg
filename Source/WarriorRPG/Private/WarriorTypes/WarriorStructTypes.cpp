// WarriorStructTypes.cpp
// Implements FHeroGameplayAbilityInfo::IsValid().
// Kept in a separate .cpp so including WarriorStructTypes.h does not pull in
// the full UWarriorHeroGameplayAbility header in every translation unit.

#include "WarriorTypes/WarriorStructTypes.h"
#include "AbilitySystem/GameplayAbilities/WarriorHeroGameplayAbility.h"

bool FHeroGameplayAbilityInfo::IsValid() const
{
    // Both fields must be set — a missing tag means the ASC can never find the ability
    // via input, and a missing class means there is nothing to grant.
    return InputTag.IsValid() && GameplayAbility != nullptr;
}
