// WarriorHeroGameplayAbility.h
// Base class for all hero-specific gameplay abilities.
// Provides cached, type-safe getters for the hero character, its controller,
// and its combat component — eliminating repetitive casting in concrete abilities.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayAbilities/WarriorGameplayAbility.h"
#include "WarriorHeroGameplayAbility.generated.h"

class AWarriorHeroCharacter;
class AWarriorHeroController;
class UHeroCombatComponent;

/**
 * Base Gameplay Ability for hero character abilities in WarriorRPG.
 * Adds cached getters for the common hero-specific objects (character, controller,
 * combat component) so concrete hero abilities can access them with minimal boilerplate.
 *
 * Caching uses TWeakObjectPtr so the cached references automatically invalidate
 * if the referenced object is destroyed (e.g., on character respawn), preventing
 * dangling pointer crashes.
 */
UCLASS()
class WARRIORRPG_API UWarriorHeroGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * Returns the hero character that owns this ability, cached on first access.
	 * The cached reference is invalidated automatically if the character is destroyed.
	 *
	 * @return The owning AWarriorHeroCharacter. Crashes in development if the avatar
	 *         is not a hero character, which would indicate a misconfiguration.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Gameplay Ability")
	AWarriorHeroCharacter* GetHeroCharacterFromActorInfo();

	/**
	 * Returns the hero's player controller, cached on first access.
	 * Use this to access player-specific state like input bindings, camera manager, HUD.
	 *
	 * @return The owning AWarriorHeroController. Crashes in development if the player
	 *         controller is not a hero controller.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Gameplay Ability")
	AWarriorHeroController* GetHeroControllerFromActorInfo();

	/**
	 * Returns the hero's combat component, resolved through the cached hero character.
	 * Not cached directly because the combat component is a subobject of the character —
	 * if the character reference is valid, so is its combat component.
	 *
	 * @return The hero's UHeroCombatComponent. Crashes in development if missing.
	 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Gameplay Ability")
	UHeroCombatComponent* GetHeroCombatComponentFromActorInfo();

	/**
 * Builds a GameplayEffectSpecHandle pre-configured for hero melee damage.
 * Sets BaseDamage via SetByCaller so the GE reads the value at application time
 * rather than using a hard-coded magnitude in the asset.
 *
 * @param InEffectClass             The Gameplay Effect class to instantiate. Must not be null.
 * @param InWeaponBaseDamage        The raw damage value from the weapon's data asset.
 * @param InCurrentAttackTypeTag    SetByCaller tag identifying the attack type (light/heavy).
 *                                  Used by the GE to apply the correct damage modifier.
 * @param InUsedComboCount          Used combo hit count, passed as the magnitude for
 *                                  InCurrentAttackTypeTag — allows damage scaling per combo step.
 * @return                          A configured FGameplayEffectSpecHandle ready to be applied.
 */
	UFUNCTION(BlueprintPure,
		Category = "Warrior|Ability")
	FGameplayEffectSpecHandle MakeHeroDamageEffectSpecHandle(TSubclassOf<UGameplayEffect> InEffectClass,
	                                                         float InWeaponBaseDamage,
	                                                         FGameplayTag InCurrentAttackTypeTag,
	                                                         int32 InUsedComboCount);

private:
	/**
	 * Cached weak reference to the owning hero character.
	 * TWeakObjectPtr automatically becomes invalid if the character is destroyed,
	 * making it safe to hold references across frames without risking dangling pointers.
	 */
	mutable TWeakObjectPtr<AWarriorHeroCharacter> CachedWarriorHeroCharacter;

	/**
	 * Cached weak reference to the owning hero controller.
	 * Same rationale as the character cache — safe against destruction between frames.
	 */
	TWeakObjectPtr<AWarriorHeroController> CachedWarriorHeroController;
};
