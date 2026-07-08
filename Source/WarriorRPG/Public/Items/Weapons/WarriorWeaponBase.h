// WarriorWeaponBase.h
// Base class for all weapons in WarriorRPG.
// Defines the shared structure: static mesh for visuals and a box collider
// for gameplay hit detection. Collision is disabled by default and toggled
// during attack animation notifies to create precise hit windows.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WarriorWeaponBase.generated.h"

class UBoxComponent;

DECLARE_DELEGATE_OneParam(FOnTargetInteractedDelegate,
                          AActor*)

/**
 * Abstract base class for all weapons in WarriorRPG.
 * Provides the shared visual mesh and hit collision box that all weapon
 * variants (hero weapons, enemy weapons) inherit.
 *
 * The collision box is disabled by default. Enable it via animation notifies
 * during the active frames of an attack to create precise hit windows —
 * never leave it always-on, or the weapon will damage enemies by proximity.
 */
UCLASS()
class WARRIORRPG_API AWarriorWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWarriorWeaponBase();

    /** Returns the weapon's hit collision box for external systems (e.g., combat manager, animation notifies). */
    FORCEINLINE UBoxComponent* GetWeaponCollisionBox() const
    {
        return WeaponCollisionBox;
    }

    /** Fired when the weapon's collision box begins overlapping a valid target pawn.
     *  Bound by UPawnCombatComponent::RegisterSpawnedWeapon to OnHitTargetActor. */
    FOnTargetInteractedDelegate OnWeaponHitTarget;

    /** Fired when the weapon's collision box stops overlapping a target pawn.
     *  Bound by UPawnCombatComponent::RegisterSpawnedWeapon to OnWeaponPulledFromTargetActor. */
    FOnTargetInteractedDelegate OnWeaponPulledFromTarget;

protected:
    /**
     * Visual mesh of the weapon. Also serves as the root component so the
     * entire weapon transforms as a single unit when attached to a socket.
     */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Weapons")
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

    /**
     * Hit detection collider. Disabled by default — enable only during
     * active attack frames via animation notifies.
     */
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Weapons")
    TObjectPtr<UBoxComponent> WeaponCollisionBox;

    /** Called when the collision box begins overlapping another actor.
     *  Filters to pawns only and ignores the weapon's own owner.
     *  Fires OnWeaponHitTarget to notify the combat component.   
     */
    UFUNCTION()
    virtual void OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                            AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp,
                                            int32 OtherBodyIndex,
                                            bool bFromSweep,
                                            const FHitResult& SweepResult);

    /** Called when the collision box stops overlapping another actor.
     *  Fires OnWeaponPulledFromTarget to notify the combat component.   
     */
    UFUNCTION()
    virtual void OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                          AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp,
                                          int32 OtherBodyIndex);
};
