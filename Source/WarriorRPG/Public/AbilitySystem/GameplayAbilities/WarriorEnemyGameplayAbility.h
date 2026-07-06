// WarriorEnemyGameplayAbility.h
// Base class for all enemy-specific gameplay abilities.
// Enemy abilities are typically AI-driven rather than input-driven, and often
// include telegraph windows, coordinated group tactics, and aggro management.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/WarriorGameplayAbility.h"
#include "WarriorEnemyGameplayAbility.generated.h"

class AWarriorEnemyCharacter;
class UEnemyCombatComponent;

/**
 * Base Gameplay Ability for enemy characters in WarriorRPG.
 * Inherits the activation policy system from UWarriorGameplayAbility and will
 * gain enemy-specific helpers (cached enemy character, AI controller access,
 * enemy combat component) as the enemy combat system develops.
 */
UCLASS()
class WARRIORRPG_API UWarriorEnemyGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * Returns the enemy character that owns this ability, cached on first access.
	 * The cached reference is invalidated automatically if the character is destroyed.
	 *
	 * @return The owning AWarriorEnemyCharacter. Crashes in development if the avatar
	 *         is not an enemy character, which indicates a misconfiguration (ability
	 *         granted to the wrong character type).
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Ability")
	AWarriorEnemyCharacter* GetEnemyCharacterFromActorInfo();

	/**
	 * Returns the enemy combat component from the cached enemy character.
	 * Resolves through GetEnemyCharacterFromActorInfo — if the character is valid,
	 * its combat component is guaranteed to exist as a default subobject.
	 *
	 * @return The owning UEnemyCombatComponent. Crashes in development if missing,
	 *         which indicates the component was not created in the constructor.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Ability")
	UEnemyCombatComponent* GetEnemyCombatComponentFromActorInfo();

	/**
	 * Builds a GameplayEffectSpecHandle pre-configured for enemy melee damage.
	 * Evaluates InDamageScalableFloat at the current ability level to produce the
	 * final damage magnitude, then injects it as a SetByCaller value under
	 * Shared::SetByCaller::BaseDamage so the GE reads it at application time.
	 *
	 * @param EffectClass               The Gameplay Effect class to instantiate. Must not be null.
	 * @param InDamageScalableFloat     Curve-based damage value evaluated at the ability's level.
	 *                                  Allows per-level damage scaling without modifying the GE asset.
	 * @return                          A configured FGameplayEffectSpecHandle ready to be applied.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Ability")
	FGameplayEffectSpecHandle MakeEnemyDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> EffectClass,
	                                                          const FScalableFloat& InDamageScalableFloat);

private:
	/**
	 * Cached weak reference to the owning enemy character.
	 * TWeakObjectPtr automatically becomes invalid if the character is destroyed,
	 * making it safe to hold references across frames without risking dangling pointers.
	 */
	TWeakObjectPtr<AWarriorEnemyCharacter> CachedWarriorEnemyCharacter;
};
