// DataAsset_StartupDataBase.h
// Base Data Asset that defines the startup abilities of a character.
// Holds two ability lists: one for abilities that auto-activate when granted,
// and one for reactive abilities that wait for external triggers.
// Subclass this per character type (hero, enemy variants) to create concrete assets.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StartupDataBase.generated.h"

class UGameplayEffect;
class UWarriorAbilitySystemComponent;
class UWarriorGameplayAbility;

/**
 * Base Data Asset for character startup configuration.
 * When a character is possessed, its assigned StartupData is loaded and
 * its abilities are granted to the character's ASC in a single batch.
 *
 * Designers populate the ability lists in the editor without touching code,
 * making the ability loadout fully data-driven.
 */
UCLASS()
class WARRIORRPG_API UDataAsset_StartupDataBase : public UDataAsset
{
    GENERATED_BODY()

public:
    /**
     * Grants all abilities in this Data Asset to the given Ability System Component.
     * Called by the owning character during PossessedBy.
     *
     * @param InASC         The ASC that will receive the abilities. Must not be null.
     * @param ApplyLevel    The level to apply to granted abilities. Defaults to 1.
     */
    virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASC,
                                              int32 ApplyLevel = 1);

protected:
    /**
     * Abilities that auto-activate the moment they are granted.
     * Use for passive listeners, always-on effects, and abilities that
     * should start executing immediately (typically ones with the OnGiven activation policy).
     */
    UPROPERTY(EditDefaultsOnly,
        Category = "Settings")
    TArray<TSubclassOf<UWarriorGameplayAbility>> ActivateOnGivenAbilities;

    /**
     * Abilities that remain dormant after being granted, waiting for external triggers.
     * Use for player-input abilities (attacks, dodges) and event-driven abilities
     * that should only activate in response to gameplay events.
     */
    UPROPERTY(EditDefaultsOnly,
        Category = "Settings")
    TArray<TSubclassOf<UWarriorGameplayAbility>> ReactiveAbilities;

    /**
     * Gameplay Effects applied to the owning character at startup.
     * Use for initializing base attribute values (health, rage, attack power)
     * via a GE_Initialize effect. Applied after abilities are granted.
     */
    UPROPERTY(EditDefaultsOnly,
        Category = "Settings")
    TArray<TSubclassOf<UGameplayEffect>> StartUpGameplayEffects;

    /**
     * Shared helper that grants every ability in the given array to the ASC.
     * Extracted to avoid code duplication between the two ability list types.
     *
     * @param InAbilities    The array of ability classes to grant.
     * @param InASC          The ASC that will receive the abilities.
     * @param ApplyLevel     The level to apply to each granted ability.
     */
    void GrantAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilities,
                        UWarriorAbilitySystemComponent* InASC,
                        int32 ApplyLevel = 1);
};
