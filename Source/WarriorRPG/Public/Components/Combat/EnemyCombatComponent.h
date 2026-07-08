// EnemyCombatComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"

/**
 * UEnemyCombatComponent
 *
 * Combat component specialized for AI-controlled enemy characters.
 * Inherits shared weapon registration and combat state from UPawnCombatComponent.
 *
 * Hit detection follows a two-stage evaluation:
 *   1. Tag check — is the defender currently blocking (Player::Status::Blocking)?
 *   2. Geometry check — is the defender actually facing the attacker (IsValidBlock)?
 * Both must pass for the hit to be routed as a successful block.
 * The Enemy::Status::Unblockable tag on the attacker bypasses both stages entirely,
 * allowing specific attacks (boss finishers, telegraphed heavy strikes) to always land.
 *
 * Enemy-specific systems (threat tables, attack telegraphing, group tactics)
 * will be added here as features are implemented.
 */
UCLASS(ClassGroup = (Combat),
    meta = (BlueprintSpawnableComponent))
class WARRIORRPG_API UEnemyCombatComponent : public UPawnCombatComponent
{
    GENERATED_BODY()

public:
    /**
     * Called when the weapon collision box hits a new target pawn during an attack swing.
     * Deduplicates hits against the same target within a single swing via OverlappedActors.
     *
     * Evaluates block conditions in order:
     *   1. If Enemy::Status::Unblockable is set on the owning pawn, skip all block checks.
     *   2. If Player::Status::Blocking is set on the target, run the geometry validation.
     *   3. If both tag and geometry checks pass, send Player::Event::SuccessfulBlock to the defender.
     *   4. Otherwise, send Shared::Event::MeleeHit to the attacker for damage application.
     *
     * @param HitActor  The actor that was hit by the weapon collision box. Must not be null.
     */
    virtual void OnHitTargetActor(AActor* HitActor) override;

protected:
    /**
     * Enables or disables the left or right hand collision box on the owning
     * AWarriorEnemyCharacter based on ToggleDamageType.
     * Called by the base class ToggleWeaponCollision when the damage type
     * is LeftHand or RightHand rather than CurrentEquippedWeapon.
     *
     * @param bShouldEnable    True to set QueryOnly collision, false to disable it.
     * @param ToggleDamageType Which hand box to toggle (LeftHand or RightHand).
     */
    virtual void ToggleBodyCollisionBoxCollision(bool bShouldEnable,
                                                 EToggleDamageType ToggleDamageType) override;
};
