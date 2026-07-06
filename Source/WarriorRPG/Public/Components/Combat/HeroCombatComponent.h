// HeroCombatComponent.h
// Combat component specialized for the player-controlled hero.
// Adds hero-specific functionality on top of the shared combat base:
// typed weapon access, combo tracking, stamina management, and attack buffering.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "HeroCombatComponent.generated.h"

class AWarriorHeroWeapon;

/**
 * Combat component for the hero character.
 * Inherits the weapon registry and combat state from UPawnCombatComponent.
 * Adds player-specific helpers that return hero-typed weapon references,
 * avoiding repeated casts in ability and animation code.
 */
UCLASS(ClassGroup=(Custom),
	meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UHeroCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	/**
	 * Returns the carried weapon associated with the given tag, cast to AWarriorHeroWeapon.
	 * Use this instead of GetCarriedWeaponByTag() when you need access to hero-specific
	 * weapon data (e.g., HeroWeaponData.WeaponAnimLayer).
	 *
	 * Crashes in development if the weapon registered under InWeaponTag is not an
	 * AWarriorHeroWeapon — this indicates the wrong weapon class was registered
	 * in the combat component, which is always a Blueprint setup error.
	 *
	 * @param InWeaponTag   The Gameplay Tag the weapon was registered under.
	 * @return              The weapon cast to AWarriorHeroWeapon.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTag) const;

	/**
	 * Returns the currently equipped weapon cast to AWarriorHeroWeapon.
	 * Convenience wrapper over GetCurrentEquippedWeapon() that avoids repeated
	 * casting in hero ability code. Returns nullptr if no weapon is equipped.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	AWarriorHeroWeapon* GetHeroCurrentEquippedWeapon() const;

	/**
	 * Returns the base damage of the currently equipped weapon evaluated at the given level.
	 * Reads from HeroWeaponData.WeaponBaseDamage — a curve that scales damage with level.
	 * Used by MakeHeroDamageEffectSpecHandle to inject the correct damage into the GE spec.
	 *
	 * @param InLevel   The ability or character level to evaluate the damage curve at.
	 * @return          The damage float at that level. Crashes if no weapon is equipped.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Warrior|Combat")
	float GetHeroCurrentEquippedWeaponDamageAtLevel(float InLevel) const;

	/**
	 * Called when the weapon collision box hits a new target pawn during an attack swing.
	 * Sends a MeleeHit gameplay event to the hero's ASC so the active attack ability
	 * can apply the damage GE to the correct target.
	 *
	 * @param HitActor  The actor that was hit by the weapon collision box.
	 */
	virtual void OnHitTargetActor(AActor* HitActor) override;

	/**
	 * Called when the weapon collision box stops overlapping a target pawn.
	 * Reserved for cleanup of per-hit state if needed in future combat features.
	 *
	 * @param InteractedActor  The actor the weapon just stopped overlapping.
	 */
	virtual void OnWeaponPulledFromTargetActor(AActor* InteractedActor) override;
};
